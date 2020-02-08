#include "redis_server.h"
#include "storage_engine.h"
#include "spdlog/spdlog.h"
#include "cxxopts.hpp"

const cxxopts::ParseResult parseCommandlineArgs(int argc, char *argv[]) noexcept {
  try {
    cxxopts::Options options("protodb1", "prototype database #1");
    options.add_options()("threads", "Number of threads",
                          cxxopts::value<size_t>()->default_value("1"));

    return options.parse(argc, argv);
  } catch (const cxxopts::OptionException& e) {
    spdlog::error("{}", e.what());
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  auto result = parseCommandlineArgs(argc, argv);

  size_t num_threads = result["threads"].as<size_t>();

  spdlog::set_level(spdlog::level::info);

  protodb1::StorageEngine storage_engine;
  protodb1::RedisServer redisServer(&storage_engine, num_threads);
  redisServer.Run();

  return 0;
}