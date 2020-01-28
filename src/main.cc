#include "redis_server.h"
#include "storage_engine.h"
#include "spdlog/spdlog.h"

int main() {
  spdlog::set_level(spdlog::level::warn);

  protodb1::StorageEngine storage_engine;
  protodb1::RedisServer redisServer(&storage_engine);
  redisServer.Run();

  return 0;
}