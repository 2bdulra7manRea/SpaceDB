#pragma once

#include "SpaceDB/types.hpp"

#include <chrono>
#include <mutex>
#include <optional>
#include <string>

namespace SpaceDB {

class Store {
 public:
  explicit Store(std::string aof_path);

  std::optional<std::string> get(const std::string& key);
  void set(const std::string& key, const std::string& value,
           std::optional<std::chrono::milliseconds> ttl = std::nullopt);
  bool del(const std::string& key);
  bool expire(const std::string& key, std::chrono::seconds seconds);
  size_t sweep();
  SnapshotData snapshot();
  size_t size();

  const std::string& aof_path() const;

 private:
  static bool expired(const Entry& entry);

  std::mutex mutex_;
  SnapshotData values_;
  std::string aof_path_;
};

}  // namespace SpaceDB