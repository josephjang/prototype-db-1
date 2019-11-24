#include <uv.h>
#include "spdlog/spdlog.h"

namespace protodb1 {
class EchoServer {
 public:
  EchoServer();
  ~EchoServer();
  void Run();

 private:
  int port = 10000;
  uv_loop_t loop;
  uv_tcp_t server;
};
}  // namespace protodb1