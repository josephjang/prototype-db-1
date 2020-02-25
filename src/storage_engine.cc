#include "storage_engine.h"

namespace protodb1 {
StorageEngine::StorageEngine() = default;
StorageEngine::~StorageEngine() = default;

const std::optional<const std::string> StorageEngine::Get(const std::string& key) {
#if RWLOCK_IN_STORAGE_ENGINE
  std::shared_lock lock(mutex_);
#endif
#if USE_LIBCUCKOO_STORAGE_ENGINE
  std::string value;
  if (main_table_.find(key, value)) {
    return value;
  }
  return std::nullopt;
#else
  auto it = main_table_.find(key);
  if (it == main_table_.end()) {
    return std::nullopt;
  }
  return it->second;
#endif
}

void StorageEngine::Set(const std::string& key, const std::string& value) {
#if RWLOCK_IN_STORAGE_ENGINE
  std::unique_lock lock(mutex_);
#endif
  main_table_.insert_or_assign(key, value);
}

size_t StorageEngine::Delete(const std::string& key) {
#if RWLOCK_IN_STORAGE_ENGINE
  std::unique_lock lock(mutex_);
#endif
  return main_table_.erase(key);
}

}  // namespace protodb1
