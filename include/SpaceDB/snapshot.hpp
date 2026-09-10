#pragma once

#include "SpaceDB/types.hpp"

#include <string>

namespace SpaceDB {

class Snapshot {
 public:
  static void save(const std::string& path, const SnapshotData& values);
};

}  // namespace SpaceDB