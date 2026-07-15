#include "server/ClientHandler.h"

#include <cstring>
#include <iostream>

#include "network/NetworkConfig.h"
#include "utils/Logger.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#endif

ClientHandler::ClientHandler(SocketType clientSocket,
                             const std::string& clientAddress,
                             Server* server)
    : clientSocket_(clientSocket),
      clientAddress_(clientAddress),
      server_(server) {}

ClientHandler::~ClientHandler() {
#ifdef _WIN32
  closesocket(clientSocket_);
#else
  close(clientSocket_);
#endif
}

void ClientHandler::handle() {
  Logger::getInstance().info("Client connected: " +
                             clientAddress_);

  while (true) {
    std::string request = readMessage();
    if (request.empty()) {
      Logger::getInstance().info("Client disconnected: " +
                                 clientAddress_);
      break;
    }

    Logger::getInstance().debug("Received from " + clientAddress_ +
                                ": " + request);

    std::string response = server_->processCommand(request);

    if (!sendMessage(response)) {
      Logger::getInstance().error("Failed to send response to " +
                                  clientAddress_);
      break;
    }
  }
}

std::string ClientHandler::readMessage() {
  char buffer[NetworkConfig::BUFFER_SIZE];
  std::memset(buffer, 0, sizeof(buffer));

  int bytesRead = recv(clientSocket_, buffer,
                       NetworkConfig::BUFFER_SIZE - 1, 0);
  if (bytesRead <= 0) {
    return "";
  }

  return std::string(buffer, bytesRead);
}

bool ClientHandler::sendMessage(const std::string& message) {
  int totalSent = 0;
  int remaining = static_cast<int>(message.size());

  while (remaining > 0) {
    int sent = send(clientSocket_, message.c_str() + totalSent,
                    remaining, 0);
    if (sent <= 0) {
      return false;
    }
    totalSent += sent;
    remaining -= sent;
  }
  return true;
}