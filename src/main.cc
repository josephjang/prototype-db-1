#include "redis_server.h"
#include "spdlog/spdlog.h"

int main() {
  spdlog::set_level(spdlog::level::warn);

  protodb1::RedisServer redisServer;
  redisServer.Run();

  return 0;
}