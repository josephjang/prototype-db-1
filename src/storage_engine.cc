#include "storage_engine.h"

namespace protodb1 {
StorageEngine::StorageEngine() {}
StorageEngine::~StorageEngine() {}

const std::optional<const std::string> StorageEngine::Get(const std::string& key) {
  auto it = main_table_.find(key);
  if (it == main_table_.end()) {
    return std::nullopt;
  }
  return it->second;
}

void StorageEngine::Set(const std::string& key, const std::string& value) {
  main_table_.insert_or_assign(key, value);
}

size_t StorageEngine::Delete(const std::string& key) {
  return main_table_.erase(key);
}

}  // namespace protodb1
