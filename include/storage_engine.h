#pragma once

#include <cstddef>

#ifdef USE_FOLLY_STORAGE_ENGINE
#include <folly/concurrency/ConcurrentHashMap.h>
#else
#include <shared_mutex>
#include <unordered_map>
#endif

namespace protodb1 {
class StorageEngine {
 public:
  StorageEngine();
  ~StorageEngine();

  const std::optional<const std::string> Get(const std::string &key);
  void Set(const std::string &key, const std::string &value);
  size_t Delete(const std::string &key);

 private:
#ifdef USE_FOLLY_STORAGE_ENGINE
  folly::ConcurrentHashMap<std::string, std::string> main_table_;
#else
  std::unordered_map<std::string, std::string> main_table_;
  std::shared_mutex mutex_;
#endif

};
}  // namespace protodb1