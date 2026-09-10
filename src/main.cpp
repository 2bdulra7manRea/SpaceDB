#include "SpaceDB/server.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
  if (argc > 1 && std::string(argv[1]) == "--help") {
    std::cout << "usage: SpaceDB [port] [data-directory]\n";
    return 0;
  }
  try {
    const int port = argc > 1 ? std::stoi(argv[1]) : 6379;
    const std::string data_dir = argc > 2 ? argv[2] : "./data";
    std::system(("mkdir -p " + data_dir).c_str());
    SpaceDB::Server server(port, data_dir);
    server.run();
  } catch (const std::exception& error) {
    std::cerr << "fatal: " << error.what() << '\n';
    return 1;
  }
}