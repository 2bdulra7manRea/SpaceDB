#include "SpaceDB/protocol.hpp"

namespace SpaceDB {

std::optional<std::vector<std::string>> Protocol::parse_request(std::string& buffer) {
  if (buffer.empty() || buffer[0] != '*') return std::nullopt;
  auto line_end = buffer.find("\r\n");
  if (line_end == std::string::npos) return std::nullopt;
  int count = std::stoi(buffer.substr(1, line_end - 1));
  size_t cursor = line_end + 2;
  std::vector<std::string> result;
  for (int i = 0; i < count; ++i) {
    if (cursor >= buffer.size() || buffer[cursor] != '$') return std::nullopt;
    auto length_end = buffer.find("\r\n", cursor);
    if (length_end == std::string::npos) return std::nullopt;
    const auto length = std::stol(buffer.substr(cursor + 1, length_end - cursor - 1));
    cursor = length_end + 2;
    if (length < 0 || buffer.size() < cursor + static_cast<size_t>(length) + 2) return std::nullopt;
    result.push_back(buffer.substr(cursor, static_cast<size_t>(length)));
    cursor += static_cast<size_t>(length) + 2;
  }
  buffer.erase(0, cursor);
  return result;
}

std::string Protocol::simple_error(const std::string& message) { return "-ERR " + message + "\r\n"; }

std::string Protocol::integer_reply(long long value) { return ":" + std::to_string(value) + "\r\n"; }

std::string Protocol::bulk_reply(const std::optional<std::string>& value) {
  if (!value) return "$-1\r\n";
  return "$" + std::to_string(value->size()) + "\r\n" + *value + "\r\n";
}

}  // namespace SpaceDB