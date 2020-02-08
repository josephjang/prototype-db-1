#pragma once

#include <uv.h>
#include "spdlog/spdlog.h"

#include "storage_engine.h"

namespace protodb1 {
class RedisServer {
 public:
  RedisServer(protodb1::StorageEngine *storage_engine);
  ~RedisServer();
  void Run();

 private:
  static void AllocateBuffer(uv_handle_t *handle, size_t suggested_size,
                             uv_buf_t *buf);
  static void HandleClose(uv_handle_t *handle);
  static void HandleWriteData(uv_write_t *req, int status);
  static void HandleReadData(uv_stream_t *stream, ssize_t nread,
                             const uv_buf_t *buf);
  static void HandleIncomingConnection(uv_stream_t *server, int status);
  static void HandleSigTerm(uv_signal_t *handle, int signum);

  class RedisClientSession;

  static void ParseLines(RedisClientSession *session);
  static void ParseCommand(RedisClientSession *session);
  static void HandleCommand(RedisClientSession *session, std::vector<std::string> command_array);
  static void AddResponse(RedisClientSession *session, const std::string &response);
  static void WriteResponse(RedisClientSession *session);

  static void RunServer(void *arg);

  protodb1::StorageEngine *storage_engine_;
  int port = 6379;
  uv_loop_t loop;

  class RedisClientSession {
   public:
    RedisClientSession(uv_tcp_t *tcp_handle, RedisServer *server) {
      this->tcp_handle = tcp_handle;
      this->fd = tcp_handle->io_watcher.fd;
      this->server_ = server;
    }

    int GetFileDescriptor() { return fd; }
    uv_tcp_t *GetTCPHandle() { return tcp_handle; }
    RedisServer *GetServer() { return server_; }

    std::string incoming_buffer;
    std::string outgoing_buffer;
    std::vector<std::string> lines;

   private:
    int fd;
    uv_tcp_t *tcp_handle;
    RedisServer *server_;
  };
};
}  // namespace protodb1