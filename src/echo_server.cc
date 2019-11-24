#include "echo_server.h"

void AllocateBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = static_cast<char *>(malloc(suggested_size));
  if (buf->base == nullptr) {
    spdlog::error("failed to allocate a buffer with the size of {}",
                  suggested_size);
    return;
  }
  buf->len = suggested_size;
}

void HandleClose(uv_handle_t *handle) { free(handle); }

void HandleWriteData(uv_write_t *req, int status) {
  if (req->bufs != nullptr) {
    free(req->bufs);
  }

  if (status < 0) {
    spdlog::error("write failed: {}", uv_strerror(status));
    uv_close(reinterpret_cast<uv_handle_t *>(req->handle), HandleClose);
    return;
  }

  spdlog::info("write succeeded");
}

void HandleReadData(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
  if (nread <= 0 && buf->base != nullptr) {
    free(buf->base);
  }

  if (nread == UV_EOF) {
    spdlog::info("end of file: fd={}", stream->io_watcher.fd);

    uv_close(reinterpret_cast<uv_handle_t *>(stream), HandleClose);
    return;
  }

  if (nread < 0) {
    spdlog::warn("failed to read: {}", uv_strerror(nread));
    return;
  }

  if (nread == 0) {
    // EAGAIN
    spdlog::info("no data to read");
    return;
  }

  /*
    if (nread <= 0 && buf->base != NULL) {
      free(buf->base);
    }
    */

  spdlog::info("read succeeded: {} bytes", nread);

  // write back

  uv_write_t *w = static_cast<uv_write_t *>(malloc(sizeof(uv_write_t)));
  if (w == nullptr) {
    spdlog::error("failed to allocate memory");
    // ...
    return;
  }

  uv_buf_t wb = uv_buf_init(buf->base, nread);

  int r = uv_write(w, stream, &wb, 1, HandleWriteData);
  if (r < 0) {
    spdlog::error("failed to write: {}", uv_strerror(r));
  }
}

void HandleIncomingConnection(uv_stream_t *server, int status) {
  if (status < 0) {
    spdlog::error("new connection error {}", uv_strerror(status));
    return;
  }

  uv_tcp_t *client = static_cast<uv_tcp_t *>(malloc(sizeof(uv_tcp_t)));

  int r = uv_tcp_init(server->loop, client);
  if (r != 0) {
    spdlog::error(
        "failed to create a handle to accept an incoming connection: {}",
        uv_strerror(r));
    return;
  }

  r = uv_accept(server, reinterpret_cast<uv_stream_t *>(client));
  if (r != 0) {
    spdlog::error("failed to accept an incoming connection: {}",
                  uv_strerror(r));
    uv_close(reinterpret_cast<uv_handle_t *>(client), nullptr);
    return;
  }

  spdlog::info("accept succeeded: fd={}", client->io_watcher.fd);

  uv_read_start(reinterpret_cast<uv_stream_t *>(client), AllocateBuffer,
                HandleReadData);
}

protodb1::EchoServer::EchoServer() {
  int r = uv_loop_init(&loop);
  if (r != 0) {
    spdlog::error("failed to initialize libuv loop: {}", uv_strerror(r));
    throw std::runtime_error(uv_strerror(r));
  }
}

protodb1::EchoServer::~EchoServer() {
  int r = uv_loop_close(&loop);
  if (r != 0) {
    spdlog::error("failed to close libuv loop: {}", uv_strerror(r));
    // nothing we can do
  }
}

void protodb1::EchoServer::Run() {
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
}