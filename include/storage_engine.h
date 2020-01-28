#pragma once

#include <cstddef>
#include <unordered_map>

namespace protodb1 {
class StorageEngine {
 public:
  StorageEngine();
  ~StorageEngine();

  const char *Get(const char *key_bytes, size_t key_size);
  void Set(const char *key_bytes, size_t key_size, const char *value_bytes,
           size_t value_size);
  void Set(const std::string &key, const std::string &value);
  void Delete(const char *key_bytes, size_t key_size);

 private:
  std::unordered_map<std::string, std::string> main_table_;
  };
}  // namespace protodb1