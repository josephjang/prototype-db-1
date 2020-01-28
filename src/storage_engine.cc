#include "storage_engine.h"

namespace protodb1 {
StorageEngine::StorageEngine() {}
StorageEngine::~StorageEngine() {}

void StorageEngine::Get(const char *key_bytes, size_t key_size) {}

void StorageEngine::Set(const char *key_bytes, size_t key_size, const char *value_bytes,
                        size_t value_size) {}
void StorageEngine::Delete(const char *key_bytes, size_t key_size) {}

}  // namespace protodb1
