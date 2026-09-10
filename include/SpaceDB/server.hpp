#pragma once

#include "SpaceDB/aof.hpp"
#include "SpaceDB/store.hpp"

#include <atomic>
#include <string>
#include <vector>

namespace SpaceDB {

class Server {
 public:
  Server(int port, std::string data_dir);
  void run();

 private:
  void replay_aof();
  std::string execute(const std::vector<std::string>& command, bool persist = true);
  void client_loop(int client);
  void sweeper();

  int port_;
  std::string data_dir_;
  Store store_;
  AppendOnlyLog aof_;
  std::atomic<bool> running_{true};
};

}  // namespace SpaceDB