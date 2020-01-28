#include "redis_server.h"

#include <sstream>
#include <string>

void protodb1::RedisServer::AllocateBuffer(uv_handle_t *handle,
                                           size_t suggested_size,
                                           uv_buf_t *buf) {
  buf->base = static_cast<char *>(malloc(suggested_size));
  if (buf->base == nullptr) {
    spdlog::error("failed to allocate a buffer with the size of {}",
                  suggested_size);
    return;
  }
  buf->len = suggested_size;
}

void protodb1::RedisServer::HandleClose(uv_handle_t *handle) {
  auto session = static_cast<RedisClientSession *>(handle->data);
  if (session != nullptr) {
    spdlog::info("closed session: fd={}", session->GetFileDescriptor());
    delete session;
  }
  free(handle);
}

void protodb1::RedisServer::HandleWriteData(uv_write_t *req, int status) {
  auto session = static_cast<RedisClientSession *>(req->handle->data);

  if (req->bufs != nullptr) {
    free(req->bufs);
  }

  if (status < 0) {
    spdlog::error("write failed: fd={} ({})", session->GetFileDescriptor(),
                  uv_strerror(status));
    if (!uv_is_closing(reinterpret_cast<uv_handle_t *>(req->handle))) {
      uv_close(reinterpret_cast<uv_handle_t *>(req->handle), HandleClose);
    }
    free(req);
    return;
  }

  free(req);
}

void protodb1::RedisServer::HandleReadData(uv_stream_t *stream, ssize_t nread,
                                           const uv_buf_t *buf) {
  auto session = static_cast<RedisClientSession *>(stream->data);

  if (nread <= 0 && buf->base != nullptr) {
    free(buf->base);
  }

  if (nread == UV_EOF) {
    spdlog::info("end of file: fd={}", session->GetFileDescriptor());

    uv_close(reinterpret_cast<uv_handle_t *>(stream), HandleClose);
    return;
  }

  if (nread < 0) {
    spdlog::warn("failed to read: fd={} ({})", session->GetFileDescriptor(),
                 uv_strerror(nread));
    return;
  }

  if (nread == 0) {
    // EAGAIN
    spdlog::info("no data to read: fd={}", session->GetFileDescriptor());
    return;
  }

  spdlog::debug("read succeeded: fd={} ({} bytes)", session->GetFileDescriptor(),
               nread);

  session->incoming_buffer.append(buf->base, nread);
  free(buf->base);

  spdlog::debug("session incoming buffer: {} bytes",
                session->incoming_buffer.length());

  ParseLines(session);
}

void protodb1::RedisServer::ParseLines (RedisClientSession *session) {
  auto pos = session->incoming_buffer.find_first_of("\r\n");

  while (pos != std::string::npos) {
    const auto& line = session->incoming_buffer.substr(0, pos);
    SPDLOG_DEBUG("session line parsed: {} bytes", line.length());
    auto next_cmd_pos = session->incoming_buffer.find_first_not_of("\r\n", pos);
    session->incoming_buffer.erase(0, next_cmd_pos);
    SPDLOG_DEBUG("session incoming buffer remaining after truncated: {} bytes",
                  session->incoming_buffer.length());
    // extra copy
    session->lines.push_back(line);
    pos = session->incoming_buffer.find_first_of("\r\n");
  }
  ParseCommand(session);
}

void protodb1::RedisServer::ParseCommand(RedisClientSession *session) {
  bool array_started = false;
  int array_length_remaining = -1;
  bool bulk_string_expected = false;
  int bulk_string_length_expected = -1;

  auto command_array = std::vector<std::string>();
  auto pos_to_be_erased = session->lines.begin();

  for (auto it = session->lines.begin(); it != session->lines.end(); ++it) {
    const auto& line = *it;
    if (line.length() > 0) {
      if (line[0] == '*') {
        array_started = true;
        array_length_remaining = std::stoi(line.substr(1));
        // TODO: error handling
        spdlog::debug("session array starts: {} elements", array_length_remaining);
      } else if (line[0] == '$') {
        bulk_string_expected = true;
        bulk_string_length_expected = std::stoi(line.substr(1));
        // TODO: error handling
        spdlog::debug("session bulk string expected: {} chars", bulk_string_length_expected);
      } else {
        if (bulk_string_expected) {
          if (line.length() != bulk_string_length_expected) {
            spdlog::warn(
                "line does not match the expected bulk string length: "
                "line='{}', length={}",
                line, bulk_string_length_expected);
            // TODO: consider disconnect
          }
          command_array.push_back(line);

          spdlog::debug("session bulk string fulfilled: {} chars",
                       bulk_string_length_expected);
          bulk_string_expected = false;
          bulk_string_length_expected = -1;
        } else {
          command_array.push_back(line);
        }
        
        if (array_started && --array_length_remaining <= 0) {
          HandleCommand(session, command_array);
          command_array.clear();

          pos_to_be_erased = it + 1;

          spdlog::debug("session array finishes");
          array_started = false;
          array_length_remaining = -1;
        } else if (!array_started) {
          HandleCommand(session, command_array);
          command_array.clear();

          pos_to_be_erased = it + 1;
        }
      }
    }
  }

  session->lines.erase(session->lines.begin(), pos_to_be_erased);
  spdlog::debug("session line buffer truncated: {} lines remaining",
               session->lines.size());
}

void protodb1::RedisServer::HandleCommand(RedisClientSession *session,
                                          std::vector<std::string> command_array) {
  auto command = command_array[0];
  spdlog::info("command: '{}' ({} bytes)", command, command.length());
  for (auto it = command_array.begin() + 1; it != command_array.end(); it++) {
    spdlog::info("\targument: '{}' ({} bytes)", *it, (*it).length());
  }

  uv_stream_t *stream =
      reinterpret_cast<uv_stream_t *>(session->GetTCPHandle());

  if (command == "PING") {
    WriteResponse(stream, "+PONG\r\n");
  } else if (command == "SET") {
    session->GetServer()->storage_engine_->Set(command_array[1].c_str(), command_array[1].length(), command_array[2].c_str(), command_array[2].length());
    WriteResponse(stream, "+OK\r\n");
  } else {
    char resp_buf[1024];
    auto resp_len = snprintf(resp_buf, 1024, "-ERR unknown command '%s'\r\n",
                             command.c_str());
    WriteResponse(stream, resp_buf);
  }
}

void protodb1::RedisServer::WriteResponse(uv_stream_t *stream,
                                          const char *str) {
  auto session = static_cast<RedisClientSession *>(stream->data);

  uv_buf_t wb;
  wb.len = strlen(str);
  wb.base = static_cast<char *>(malloc(wb.len));
  if (wb.base == nullptr) {
    spdlog::error("failed to allocate a buffer with the size of {}", wb.len);
    return;
  }
  memcpy(wb.base, str, wb.len);

  /*
    int r = uv_try_write(stream, &wb, 1);
    if (r == wb.len) {
      spdlog::info("succeeded to try to write: fd={}",
                   session->GetFileDescriptor());
      free(wb.base);
      return;
    }
    spdlog::warn("failed to try to write: {}", uv_strerror(r));
    */

  uv_write_t *w = static_cast<uv_write_t *>(malloc(sizeof(uv_write_t)));
  if (w == nullptr) {
    spdlog::error("failed to allocate memory");
    // ...
    return;
  }

  int r = uv_write(w, stream, &wb, 1, HandleWriteData);
  if (r < 0) {
    spdlog::error("failed to write: fd={} ({})", session->GetFileDescriptor(),
                  uv_strerror(r));
    free(w);
  }
}

void protodb1::RedisServer::HandleIncomingConnection(uv_stream_t *server_handle,
                                                     int status) {
  if (status < 0) {
    spdlog::error("new connection error {}", uv_strerror(status));
    return;
  }

  uv_tcp_t *client = static_cast<uv_tcp_t *>(malloc(sizeof(uv_tcp_t)));

  int r = uv_tcp_init(server_handle->loop, client);
  if (r != 0) {
    spdlog::error(
        "failed to create a handle to accept an incoming connection: {}",
        uv_strerror(r));
    return;
  }

  r = uv_accept(server_handle, reinterpret_cast<uv_stream_t *>(client));
  if (r != 0) {
    spdlog::error("failed to accept an incoming connection: {}",
                  uv_strerror(r));
    uv_close(reinterpret_cast<uv_handle_t *>(client), nullptr);
    return;
  }

  auto server = reinterpret_cast<RedisServer *>(server_handle->data);

  auto session = new RedisClientSession(client, server);
  client->data = session;

  uv_stream_set_blocking(reinterpret_cast<uv_stream_t *>(client), 0);

  spdlog::info("created session: fd={}", session->GetFileDescriptor());

  uv_read_start(reinterpret_cast<uv_stream_t *>(client), AllocateBuffer,
                HandleReadData);
}

protodb1::RedisServer::RedisServer(protodb1::StorageEngine *storage_engine) {
  storage_engine_ = storage_engine;
  
  int r = uv_loop_init(&loop);
  if (r != 0) {
    spdlog::error("failed to initialize libuv loop: {}", uv_strerror(r));
    throw std::runtime_error(uv_strerror(r));
  }
}

protodb1::RedisServer::~RedisServer() {
  /*
  while (uv_loop_alive(&loop)) {
    spdlog::warn("libuv loop is still alive");
    sleep(1);
  }

  int r = uv_loop_close(&loop);
  if (r != 0) {
    spdlog::error("failed to close libuv loop: {}", uv_strerror(r));
    // nothing we can do
  }
  */
  spdlog::warn("destroying redis server");
}

void protodb1::RedisServer::HandleSigTerm(uv_signal_t *handle, int signum) {
  spdlog::warn("Signal received: {}", signum);
  uv_signal_stop(handle);

  uv_stop(handle->loop);
}

void protodb1::RedisServer::Run() {
  uv_signal_t sigint;
  uv_signal_init(&loop, &sigint);
  uv_signal_start(&sigint, HandleSigTerm, SIGINT);

  uv_signal_t sigterm;
  uv_signal_init(&loop, &sigterm);
  uv_signal_start(&sigterm, HandleSigTerm, SIGTERM);

  int r = uv_tcp_init(&loop, &server);
  if (r != 0) {
    spdlog::error("failed to initialize libuv tcp handle: {}", uv_strerror(r));
    throw std::runtime_error(uv_strerror(r));
  }

  sockaddr_in addr;
  r = uv_ip4_addr("0.0.0.0", port, &addr);
  if (r != 0) {
    spdlog::error("failed to convert server address: {}", uv_strerror(r));
    throw std::runtime_error(uv_strerror(r));
  }

  r = uv_tcp_bind(&server, reinterpret_cast<const struct sockaddr *>(&addr), 0);
  if (r != 0) {
    spdlog::error("failed to bind server: {}", uv_strerror(r));
    throw std::runtime_error(uv_strerror(r));
  }

  r = uv_listen(reinterpret_cast<uv_stream_t *>(&server), 100,
                HandleIncomingConnection);
  if (r != 0) {
    spdlog::error("failed to listen server: {}", uv_strerror(r));
    throw std::runtime_error(uv_strerror(r));
  }

  uv_run(&loop, UV_RUN_DEFAULT);
  spdlog::warn("uv loop is finished");

  uv_close(reinterpret_cast<uv_handle_t *>(&server), HandleClose);
}