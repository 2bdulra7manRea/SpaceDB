#include "SpaceDB/snapshot.hpp"

#include <cstdio>
#include <fstream>
#include <stdexcept>

namespace SpaceDB {

void Snapshot::save(const std::string& path, const SnapshotData& values) {
  const std::string temp = path + ".tmp";
  std::ofstream out(temp, std::ios::trunc);
  if (!out) throw std::runtime_error("cannot open snapshot: " + temp);
  const auto now = SystemClock::now();
  for (const auto& [key, entry] : values) {
    long long ttl = -1;
    if (entry.expires_at) {
      ttl = std::chrono::duration_cast<std::chrono::milliseconds>(*entry.expires_at - now).count();
      if (ttl <= 0) continue;
    }
    out << key.size() << ' ' << entry.value.size() << ' ' << ttl << '\n' << key << entry.value;
  }
  out.close();
  if (std::rename(temp.c_str(), path.c_str()) != 0) throw std::runtime_error("cannot replace snapshot");
}

}  // namespace SpaceDB