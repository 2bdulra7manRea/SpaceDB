#pragma once

#include <chrono>
#include <map>
#include <optional>
#include <string>

namespace SpaceDB {

using SystemClock = std::chrono::system_clock;

// A value owns its optional absolute expiration deadline.
struct Entry {
  std::string value;
  std::optional<SystemClock::time_point> expires_at;
};

using SnapshotData = std::map<std::string, Entry>;

}  // namespace SpaceDB