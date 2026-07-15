#ifndef SERVER_H
#define SERVER_H

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "devices/Device.h"
#include "devices/Light.h"
#include "devices/SecurityCamera.h"
#include "devices/Thermostat.h"
#include "network/ARPTable.h"
#include "network/RoutingTable.h"
#include "network/SubnetManager.h"

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

class Server {
 public:
  Server(int port);
  ~Server();

  // Start the server (blocking)
  void start();

  // Stop the server gracefully
  void stop();

  // Initialize all smart home devices
  void initializeDevices();

  // Initialize network (subnets, routing, ARP) without starting socket
  void initializeNetwork();

  // Process an incoming command and return a response
  std::string processCommand(const std::string& rawRequest);

 private:
  int port_;
  SocketType serverSocket_;
  std::atomic<bool> running_;

  // Network components
  SubnetManager subnetManager_;
  RoutingTable routingTable_;
  ARPTable arpTable_;

  // Device registry: device_id -> Device pointer
  std::map<std::string, std::shared_ptr<Device>> devices_;
  mutable std::mutex devicesMutex_;

  // Socket helpers
  bool initSocket();
  void acceptClients();
  void cleanupSocket();

  // Command handlers
  std::string handleLightCommand(
      const std::vector<std::string>& segments);
  std::string handleThermostatCommand(
      const std::vector<std::string>& segments);
  std::string handleSecurityCommand(
      const std::vector<std::string>& segments);
  std::string handleDeviceListCommand();
  std::string handlePingCommand(const std::string& targetIP);
  std::string handleArpCommand(const std::string& targetIP);
};

#endif  // SERVER_H