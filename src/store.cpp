#include "SpaceDB/store.hpp"

namespace SpaceDB {

Store::Store(std::string aof_path) : aof_path_(std::move(aof_path)) {}

std::optional<std::string> Store::get(const std::string& key) {
  std::lock_guard lock(mutex_);
  auto it = values_.find(key);
  if (it == values_.end()) return std::nullopt;
  if (expired(it->second)) {
    values_.erase(it);
    return std::nullopt;
  }
  return it->second.value;
}

void Store::set(const std::string& key, const std::string& value,
                std::optional<std::chrono::milliseconds> ttl) {
  std::lock_guard lock(mutex_);
  Entry entry{value, std::nullopt};
  if (ttl) entry.expires_at = SystemClock::now() + *ttl;
  values_[key] = std::move(entry);
}

bool Store::del(const std::string& key) {
  std::lock_guard lock(mutex_);
  return values_.erase(key) != 0;
}

bool Store::expire(const std::string& key, std::chrono::seconds seconds) {
  std::lock_guard lock(mutex_);
  auto it = values_.find(key);
  if (it == values_.end() || expired(it->second)) {
    if (it != values_.end()) values_.erase(it);
    return false;
  }
  it->second.expires_at = SystemClock::now() + seconds;
  return true;
}

size_t Store::sweep() {
  std::lock_guard lock(mutex_);
  size_t removed = 0;
  for (auto it = values_.begin(); it != values_.end();) {
    if (expired(it->second)) {
      it = values_.erase(it);
      ++removed;
    } else {
      ++it;
    }
  }
  return removed;
}

SnapshotData Store::snapshot() {
  std::lock_guard lock(mutex_);
  SnapshotData result;
  for (const auto& [key, entry] : values_) {
    if (!expired(entry)) result.emplace(key, entry);
  }
  return result;
}

size_t Store::size() {
  sweep();
  std::lock_guard lock(mutex_);
  return values_.size();
}

const std::string& Store::aof_path() const { return aof_path_; }

bool Store::expired(const Entry& entry) {
  return entry.expires_at && SystemClock::now() >= *entry.expires_at;
}

}  // namespace SpaceDB