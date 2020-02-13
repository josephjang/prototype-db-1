#pragma once

#include <cstddef>

#define USE_FOLLY_STORAGE_ENGINE 0
#define USE_LIBCUCKOO_STORAGE_ENGINE 1

#if USE_FOLLY_STORAGE_ENGINE || USE_LIBCUCKOO_STORAGE_ENGINE
#define RWLOCK_IN_STORAGE_ENGINE 0
#else
#define RWLOCK_IN_STORAGE_ENGINE 1
#endif

#if USE_FOLLY_STORAGE_ENGINE
#include <folly/concurrency/ConcurrentHashMap.h>
#elif USE_LIBCUCKOO_STORAGE_ENGINE
#include "libcuckoo/cuckoohash_map.hh"
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
#if USE_FOLLY_STORAGE_ENGINE
  folly::ConcurrentHashMap<std::string, std::string> main_table_;
#elif USE_LIBCUCKOO_STORAGE_ENGINE
  libcuckoo::cuckoohash_map<std::string, std::string> main_table_;
#else
  std::unordered_map<std::string, std::string> main_table_;
  std::shared_mutex mutex_;
#endif

};
}  // namespace protodb1