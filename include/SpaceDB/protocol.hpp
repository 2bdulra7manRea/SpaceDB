#pragma once

#include <optional>
#include <string>
#include <vector>

namespace SpaceDB {

class Protocol {
 public:
  // Returns no value when the buffer contains only a partial frame.
  static std::optional<std::vector<std::string>> parse_request(std::string& buffer);
  static std::string simple_error(const std::string& message);
  static std::string integer_reply(long long value);
  static std::string bulk_reply(const std::optional<std::string>& value);
};

}  // namespace SpaceDB