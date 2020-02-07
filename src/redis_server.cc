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

void protodb1::RedisServer::ParseLines(protodb1::RedisServer::RedisClientSession *session) {
  auto start_pos = 0;
  auto pos = session->incoming_buffer.find_first_of("\r\n", start_pos);

  while (pos != std::string::npos) {
    //spdlog::debug("start: {}, pos: {}", start_pos, pos);
    const auto& line = session->incoming_buffer.substr(start_pos, pos - start_pos);
    SPDLOG_DEBUG("session line parsed: {} bytes", line.length());
    //spdlog::debug("session line parsed: {} bytes, '{}'", line.length(), line);
    session->lines.emplace_back(std::move(line));
    start_pos = session->incoming_buffer.find_first_not_of("\r\n", pos);
    pos = session->incoming_buffer.find_first_of("\r\n", start_pos);
  }
  session->incoming_buffer.erase(0, start_pos);
  SPDLOG_DEBUG("session incoming buffer remaining after truncated: {} bytes",
               session->incoming_buffer.length());
  ParseCommand(session);
}

void protodb1::RedisServer::ParseCommand(protodb1::RedisServer::RedisClientSession *session) {
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
          command_array.emplace_back(line);

          spdlog::debug("session bulk string fulfilled: {} chars",
                       bulk_string_length_expected);
          bulk_string_expected = false;
          bulk_string_length_expected = -1;
        } else {
          command_array.emplace_back(line);
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

  WriteResponse(session);

  session->lines.erase(session->lines.begin(), pos_to_be_erased);
  spdlog::debug("session line buffer truncated: {} lines remaining",
               session->lines.size());
}

void protodb1::RedisServer::HandleCommand(protodb1::RedisServer::RedisClientSession *session,
                                          std::vector<std::string> command_array) {
  static const std::string &ping_cmd = "PING";
  static const std::string &set_cmd = "SET";
  static const std::string &ok_resp = "+OK\r\n";
  static const std::string &pong_resp = "+PONG\r\n";

  const auto &command = command_array[0];
  /*
  spdlog::info("command: '{}' ({} bytes)", command, command.length());
  for (auto it = command_array.begin() + 1; it != command_array.end(); it++) {
    spdlog::info("\targument: '{}' ({} bytes)", *it, (*it).length());
  }
  */

  if (command == ping_cmd) {
    AddResponse(session, pong_resp);
  } else if (command == set_cmd) {
    session->GetServer()->storage_engine_->Set(command_array[1], command_array[2]);
    AddResponse(session, ok_resp);
  } else {
    char resp_buf[1024];
    auto resp_len = snprintf(resp_buf, 1024, "-ERR unknown command '%s'\r\n",
                             command.c_str());
    AddResponse(session, resp_buf);
  }
}

void protodb1::RedisServer::AddResponse(protodb1::RedisServer::RedisClientSession *session,
                                        const std::string &response) {
  session->outgoing_buffer.append(response);
}

void protodb1::RedisServer::WriteResponse(protodb1::RedisServer::RedisClientSession *session) {
  uv_stream_t *stream =
      reinterpret_cast<uv_stream_t *>(session->GetTCPHandle());

  uv_buf_t wb;
  wb.len = session->outgoing_buffer.length();
  wb.base = static_cast<char *>(malloc(wb.len));
  if (wb.base == nullptr) {
    spdlog::error("failed to allocate a buffer with the size of {}", wb.len);
    return;
  }
  session->outgoing_buffer.copy(wb.base, wb.len);

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

  session->outgoing_buffer.clear();
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

  for (int i = 0; i < 1; i++) {
    uv_thread_t thread_handle;
    uv_thread_create(&thread_handle, RunServer, this);
  }

  uv_run(&loop, UV_RUN_DEFAULT);
  spdlog::warn("uv loop is finished");

  uv_close(reinterpret_cast<uv_handle_t *>(&server), HandleClose);
}

void protodb1::RedisServer::RunServer(void *arg) {
  uv_loop_t loop;
  uv_tcp_t server_handle;

  uv_loop_init(&loop);

  int r = uv_tcp_init_ex(&loop, &server_handle, AF_INET);
  if (r != 0) {
    spdlog::error("failed to initialize libuv tcp handle: {}", uv_strerror(r));
    throw std::runtime_error(uv_strerror(r));
  }

  auto server = static_cast<RedisServer *>(arg);
  server_handle.data = server;

  sockaddr_in addr;
  r = uv_ip4_addr("0.0.0.0", 6379, &addr);
  if (r != 0) {
    spdlog::error("failed to convert server address: {}", uv_strerror(r));
    throw std::runtime_error(uv_strerror(r));
  }

  uv_os_fd_t fd;
  int on = 1;
  uv_fileno(reinterpret_cast<uv_handle_t *>(&server_handle), &fd);
  r = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (char *)&on, sizeof(on));
  if (r != 0) {
    spdlog::error("failed to set the socket option SO_REUSEPORT: fd: {}, {}", fd, uv_strerror(r));
    throw std::runtime_error(uv_strerror(r));
  }

  r = uv_tcp_bind(&server_handle, reinterpret_cast<const struct sockaddr *>(&addr), 0);
  if (r != 0) {
    spdlog::error("failed to bind server: {}", uv_strerror(r));
    throw std::runtime_error(uv_strerror(r));
  }

  r = uv_listen(reinterpret_cast<uv_stream_t *>(&server_handle), 100,
                HandleIncomingConnection);
  if (r != 0) {
    spdlog::error("failed to listen server: {}", uv_strerror(r));
    throw std::runtime_error(uv_strerror(r));
  }

  uv_run(&loop, UV_RUN_DEFAULT);
  spdlog::warn("uv loop is finished");

  uv_close(reinterpret_cast<uv_handle_t *>(&server_handle), HandleClose);
}
