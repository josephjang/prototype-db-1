#pragma once

#include <cstddef>
#include <unordered_map>
#include <shared_mutex>

namespace protodb1 {
class StorageEngine {
 public:
  StorageEngine();
  ~StorageEngine();

  const std::optional<const std::string> Get(const std::string &key);
  void Set(const std::string &key, const std::string &value);
  size_t Delete(const std::string &key);

 private:
  std::unordered_map<std::string, std::string> main_table_;
  std::shared_mutex mutex_;
};
}  // namespace protodb1