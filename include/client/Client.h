#ifndef CLIENT_H
#define CLIENT_H

#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using SocketType = SOCKET;
#define INVALID_SOCK INVALID_SOCKET
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using SocketType = int;
#define INVALID_SOCK (-1)
#endif

class Client {
 public:
  Client(const std::string& serverIP, int serverPort);
  ~Client();

  // Connect to the server
  bool connectToServer();

  // Disconnect from the server
  void disconnect();

  // Send a command and receive the response
  std::string sendCommand(const std::string& command);

  // Interactive command-line loop
  void interactiveMode();

  bool isConnected() const;

 private:
  std::string serverIP_;
  int serverPort_;
  SocketType socket_;
  bool connected_;

  // Display the help menu
  void printHelp() const;
};

#endif  // CLIENT_H