#include "echo_server.h"
#include "spdlog/spdlog.h"

int main() {
  protodb1::EchoServer echoServer;
  echoServer.Run();

  return 0;
}