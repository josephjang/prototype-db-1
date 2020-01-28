#pragma once

#include <cstddef>

namespace protodb1 {
class StorageEngine {
 public:
  StorageEngine();
  ~StorageEngine();

  void Get(const char *key_bytes, size_t key_size);
  void Set(const char *key_bytes, size_t key_size, const char *value_bytes,
           size_t value_size);
  void Delete(const char *key_bytes, size_t key_size);
};
}  // namespace protodb1