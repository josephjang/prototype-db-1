#include <uv.h>
#include "spdlog/spdlog.h"

namespace protodb1 {
class RedisServer {
 public:
  RedisServer();
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
  static void HandleCommand(uv_stream_t *stream, std::vector<std::string> command_array);
  static void WriteResponse(uv_stream_t *stream, const char *str);

  int port = 6379;
  uv_loop_t loop;
  uv_tcp_t server;

  class RedisClientSession {
   public:
    RedisClientSession(uv_tcp_t *tcp_handle) {
      this->tcp_handle = tcp_handle;
      this->fd = tcp_handle->io_watcher.fd;
    }

    int GetFileDescriptor() { return fd; }
    uv_tcp_t *GetTCPHandle() { return tcp_handle; }

    std::string incoming_buffer;
    std::vector<std::string> lines;

   private:
    int fd;
    uv_tcp_t *tcp_handle;
  };
};
}  // namespace protodb1