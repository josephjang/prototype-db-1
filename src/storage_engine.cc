#include "storage_engine.h"

namespace protodb1 {
StorageEngine::StorageEngine() {}
StorageEngine::~StorageEngine() {}

const std::optional<const std::string> StorageEngine::Get(const std::string& key) {
  std::shared_lock lock(mutex_);
  auto it = main_table_.find(key);
  if (it == main_table_.end()) {
    return std::nullopt;
  }
  return it->second;
}

void StorageEngine::Set(const std::string& key, const std::string& value) {
  std::unique_lock lock(mutex_);
  main_table_.insert_or_assign(key, value);
}

size_t StorageEngine::Delete(const std::string& key) {
  std::unique_lock lock(mutex_);
  return main_table_.erase(key);
}

}  // namespace protodb1
