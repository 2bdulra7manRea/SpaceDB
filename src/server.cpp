#include "SpaceDB/server.hpp"

#include "SpaceDB/protocol.hpp"
#include "SpaceDB/snapshot.hpp"

#include <arpa/inet.h>
#include <cctype>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace SpaceDB {

Server::Server(int port, std::string data_dir)
    : port_(port), data_dir_(std::move(data_dir)), store_(data_dir_ + "/appendonly.aof"), aof_(store_.aof_path()) {}

void Server::run() {
  replay_aof();
  const int listener = socket(AF_INET, SOCK_STREAM, 0);
  if (listener < 0) throw std::runtime_error("socket failed");
  int reuse = 1;
  setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = htons(static_cast<uint16_t>(port_));
  address.sin_addr.s_addr = INADDR_ANY;
  if (bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0 || listen(listener, 128) < 0)
    throw std::runtime_error("bind/listen failed");
  std::cout << "SpaceDB listening on 127.0.0.1:" << port_ << "\n";
  std::thread([this] { sweeper(); }).detach();
  while (running_) {
    const int client = accept(listener, nullptr, nullptr);
    if (client >= 0) std::thread(&Server::client_loop, this, client).detach();
  }
  close(listener);
}

void Server::replay_aof() { aof_.replay([this](const auto& command) { execute(command, false); }); }

std::string Server::execute(const std::vector<std::string>& command, bool persist) {
  if (command.empty()) return Protocol::simple_error("empty command");
  std::string op = command[0];
  for (char& c : op) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  if (op == "PING") return "+PONG\r\n";
  if (op == "GET" && command.size() == 2) return Protocol::bulk_reply(store_.get(command[1]));
  if (op == "SET" && (command.size() == 3 || command.size() == 5)) {
    std::optional<std::chrono::milliseconds> ttl;
    if (command.size() == 5) {
      if (command[3] != "PX") return Protocol::simple_error("SET only supports PX");
      ttl = std::chrono::milliseconds(std::stoll(command[4]));
    }
    store_.set(command[1], command[2], ttl);
    if (persist) aof_.append(command);
    return "+OK\r\n";
  }
  if (op == "DEL" && command.size() == 2) {
    const auto removed = store_.del(command[1]);
    if (persist) aof_.append(command);
    return Protocol::integer_reply(removed ? 1 : 0);
  }
  if (op == "EXPIRE" && command.size() == 3) {
    const auto changed = store_.expire(command[1], std::chrono::seconds(std::stoll(command[2])));
    if (changed && persist) aof_.append(command);
    return Protocol::integer_reply(changed ? 1 : 0);
  }
  if (op == "SAVE" && command.size() == 1) {
    Snapshot::save(data_dir_ + "/dump.rdbx", store_.snapshot());
    return "+OK\r\n";
  }
  if (op == "INFO" && command.size() == 1) return Protocol::bulk_reply("keys:" + std::to_string(store_.size()));
  return Protocol::simple_error("wrong number of arguments or unknown command");
}

void Server::client_loop(int client) {
  std::string buffer;
  char chunk[4096];
  while (true) {
    const ssize_t received = recv(client, chunk, sizeof(chunk), 0);
    if (received <= 0) break;
    buffer.append(chunk, static_cast<size_t>(received));
    while (auto request = Protocol::parse_request(buffer)) {
      const std::string response = execute(*request);
      if (send(client, response.data(), response.size(), MSG_NOSIGNAL) < 0) {
        close(client);
        return;
      }
    }
  }
  close(client);
}

void Server::sweeper() {
  while (running_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    store_.sweep();
  }
}

}  // namespace SpaceDB