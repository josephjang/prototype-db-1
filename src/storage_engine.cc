#include "storage_engine.h"

namespace protodb1 {
StorageEngine::StorageEngine() {}
StorageEngine::~StorageEngine() {}

const char *StorageEngine::Get(const char *key_bytes, size_t key_size) {
  return main_table_[key_bytes].c_str();
}

void StorageEngine::Set(const char *key_bytes, size_t key_size,
                        const char *value_bytes, size_t value_size) {
  main_table_[key_bytes] = value_bytes;
}

void StorageEngine::Delete(const char *key_bytes, size_t key_size) {
  main_table_.erase(key_bytes);
}

}  // namespace protodb1
