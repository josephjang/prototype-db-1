#include "storage_engine.h"

namespace protodb1 {
StorageEngine::StorageEngine() {}
StorageEngine::~StorageEngine() {}

const std::string& StorageEngine::Get(const std::string& key) {
  return main_table_[key];
}

void StorageEngine::Set(const std::string& key, const std::string& value) {
  main_table_.emplace(key, value);
}

size_t StorageEngine::Delete(const std::string& key) {
  return main_table_.erase(key);
}

}  // namespace protodb1
