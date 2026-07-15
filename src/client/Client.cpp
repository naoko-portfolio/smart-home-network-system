#include "client/Client.h"

#include <cstring>
#include <iostream>

#include "network/NetworkConfig.h"
#include "protocol/HttpProtocol.h"

Client::Client(const std::string& serverIP, int serverPort)
    : serverIP_(serverIP),
      serverPort_(serverPort),
      socket_(INVALID_SOCK),
      connected_(false) {}

Client::~Client() { disconnect(); }

bool Client::connectToServer() {
#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    std::cerr << "WSAStartup failed" << std::endl;
    return false;
  }
#endif

  socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_ == INVALID_SOCK) {
    std::cerr << "Failed to create socket" << std::endl;
    return false;
  }

  struct sockaddr_in serverAddr {};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(serverPort_);
  inet_pton(AF_INET, serverIP_.c_str(), &serverAddr.sin_addr);

  if (connect(socket_,
              reinterpret_cast<struct sockaddr*>(&serverAddr),
              sizeof(serverAddr)) < 0) {
    std::cerr << "Connection failed" << std::endl;
    return false;
  }

  connected_ = true;
  std::cout << "Connected to server at " << serverIP_ << ":"
            << serverPort_ << std::endl;
  return true;
}

void Client::disconnect() {
  if (connected_) {
#ifdef _WIN32
    closesocket(socket_);
    WSACleanup();
#else
    close(socket_);
#endif
    connected_ = false;
    std::cout << "Disconnected from server" << std::endl;
  }
}

std::string Client::sendCommand(const std::string& command) {
  if (!connected_) {
    return "Error: Not connected to server";
  }

  std::string request =
      HttpProtocol::buildRequest("GET", command);

  int sent =
      send(socket_, request.c_str(),
           static_cast<int>(request.size()), 0);
  if (sent <= 0) {
    connected_ = false;
    return "Error: Failed to send command";
  }

  char buffer[NetworkConfig::BUFFER_SIZE];
  std::memset(buffer, 0, sizeof(buffer));
  int bytesRead =
      recv(socket_, buffer, NetworkConfig::BUFFER_SIZE - 1, 0);
  if (bytesRead <= 0) {
    connected_ = false;
    return "Error: Failed to receive response";
  }

  return std::string(buffer, bytesRead);
}

void Client::interactiveMode() {
  if (!connected_) {
    std::cerr << "Not connected to server. Call connectToServer() "
                 "first."
              << std::endl;
    return;
  }

  printHelp();
  std::string input;

  while (true) {
    std::cout << "\nSmartHome> ";
    std::getline(std::cin, input);

    if (input == "quit" || input == "exit") {
      std::cout << "Goodbye!" << std::endl;
      break;
    }

    if (input == "help") {
      printHelp();
      continue;
    }

    if (input.empty()) {
      continue;
    }

    std::string response = sendCommand(input);
    std::cout << response << std::endl;

    if (!connected_) {
      std::cerr << "Lost connection to server." << std::endl;
      break;
    }
  }
}

bool Client::isConnected() const { return connected_; }

void Client::printHelp() const {
  std::cout
      << "\n========= Smart Home Commands =========\n"
      << "  /light/status         - Get light status\n"
      << "  /light/on             - Turn light on\n"
      << "  /light/off            - Turn light off\n"
      << "  /light/brightness/50  - Set brightness (0-100)\n"
      << "  /thermostat/status    - Get thermostat status\n"
      << "  /thermostat/set/22    - Set temperature\n"
      << "  /security/status      - Get security status\n"
      << "  /security/arm         - Arm security system\n"
      << "  /security/disarm      - Disarm security system\n"
      << "  /devices/list         - List all devices\n"
      << "  PING <ip>             - Ping an IP address\n"
      << "  ARP <ip>              - ARP lookup\n"
      << "  help                  - Show this menu\n"
      << "  quit                  - Exit\n"
      << "========================================\n";
}