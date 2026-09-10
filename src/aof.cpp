#include "SpaceDB/aof.hpp"

#include <fstream>
#include <mutex>
#include <sstream>
#include <stdexcept>

namespace SpaceDB {

AppendOnlyLog::AppendOnlyLog(std::string path) : path_(std::move(path)) {}

void AppendOnlyLog::append(const std::vector<std::string>& command) {
  std::lock_guard lock(mutex_);
  std::ofstream out(path_, std::ios::app);
  if (!out) throw std::runtime_error("cannot open AOF: " + path_);
  for (size_t i = 0; i < command.size(); ++i) {
    if (i) out << '\t';
    out << escape(command[i]);
  }
  out << '\n';
  out.flush();
}

void AppendOnlyLog::replay(const ReplayCallback& apply) {
  std::ifstream in(path_);
  if (!in) return;
  std::string line;
  while (std::getline(in, line)) {
    std::vector<std::string> command;
    std::stringstream stream(line);
    std::string field;
    while (std::getline(stream, field, '\t')) command.push_back(unescape(field));
    if (!command.empty()) apply(command);
  }
}

std::string AppendOnlyLog::escape(const std::string& value) {
  std::string result;
  for (char c : value) {
    if (c == '\\' || c == '\t' || c == '\n') result += '\\';
    result += c;
  }
  return result;
}

std::string AppendOnlyLog::unescape(const std::string& value) {
  std::string result;
  bool escaped = false;
  for (char c : value) {
    if (escaped) {
      result += c;
      escaped = false;
    } else if (c == '\\') {
      escaped = true;
    } else {
      result += c;
    }
  }
  return result;
}

// The template implementation lives in the header so callers can supply any callback.

}  // namespace SpaceDB