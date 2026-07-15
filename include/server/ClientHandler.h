#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <string>

#include "server/Server.h"

#ifdef _WIN32
#include <winsock2.h>
using SocketType = SOCKET;
#else
using SocketType = int;
#endif

// Handles a single client connection in its own thread
class ClientHandler {
 public:
  ClientHandler(SocketType clientSocket,
                const std::string& clientAddress, Server* server);
  ~ClientHandler();

  // Main loop: read requests, process, send responses
  void handle();

 private:
  SocketType clientSocket_;
  std::string clientAddress_;
  Server* server_;

  // Read a complete message from the socket
  std::string readMessage();

  // Send a message through the socket
  bool sendMessage(const std::string& message);
};

#endif  // CLIENT_HANDLER_H