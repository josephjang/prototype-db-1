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

void StorageEngine::Delete(const char *key_bytes, size_t key_size) {
  main_table_.erase(key_bytes);
}

}  // namespace protodb1
