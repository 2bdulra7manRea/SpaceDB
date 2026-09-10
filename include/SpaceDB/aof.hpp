#pragma once

#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace SpaceDB {

class AppendOnlyLog {
 public:
  explicit AppendOnlyLog(std::string path);

  void append(const std::vector<std::string>& command);

  using ReplayCallback = std::function<void(const std::vector<std::string>&)>;
  void replay(const ReplayCallback& apply);

 private:
  static std::string escape(const std::string& value);
  static std::string unescape(const std::string& value);

  std::mutex mutex_;
  std::string path_;
};

}  // namespace SpaceDB