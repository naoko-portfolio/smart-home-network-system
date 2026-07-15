#include "server/Server.h"

#include <cstring>
#include <iostream>
#include <sstream>
#include <thread>

#include "protocol/HttpProtocol.h"
#include "protocol/ICMPHandler.h"
#include "server/ClientHandler.h"
#include "utils/Logger.h"
#include "utils/ThreadPool.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

Server::Server(int port)
    : port_(port), serverSocket_(INVALID_SOCK), running_(false) {}

Server::~Server() { stop(); }

void Server::initializeNetwork() {
  subnetManager_.initializeSubnets();

  auto subnets = NetworkConfig::getSubnetConfigurations();
  for (const auto& s : subnets) {
    routingTable_.addRoute(s.networkAddress, s.cidr, s.gateway,
                           s.name);
  }
  routingTable_.setDefaultGateway("192.168.1.1", "Management");
}

void Server::start() {
  Logger::getInstance().info("Server starting on port " +
                             std::to_string(port_));

  // Initialize network
  initializeNetwork();

  // Initialize devices
  initializeDevices();

  // Print network info
  subnetManager_.printSubnetAllocations();
  routingTable_.printTable();
  arpTable_.printTable();

  if (!initSocket()) {
    Logger::getInstance().error("Failed to initialize socket");
    return;
  }

  running_ = true;
  Logger::getInstance().info("Server is running. Waiting for "
                             "connections...");
  acceptClients();
}

void Server::stop() {
  running_ = false;
  cleanupSocket();
  Logger::getInstance().info("Server stopped");
}

void Server::initializeDevices() {
  // Create lights in the Lighting subnet
  auto lightIP1 = subnetManager_.allocateIP("Lighting");
  auto light1 = std::make_shared<Light>(
      "light-1", "Living Room Light", lightIP1,
      arpTable_.arpRequest(lightIP1), "Lighting");
  devices_["light"] = light1;

  auto lightIP2 = subnetManager_.allocateIP("Lighting");
  auto light2 = std::make_shared<Light>(
      "light-2", "Bedroom Light", lightIP2,
      arpTable_.arpRequest(lightIP2), "Lighting");
  devices_["light2"] = light2;

  // Create thermostat in the Thermostat subnet
  auto thermoIP = subnetManager_.allocateIP("Thermostat");
  auto thermo = std::make_shared<Thermostat>(
      "thermo-1", "Main Thermostat", thermoIP,
      arpTable_.arpRequest(thermoIP), "Thermostat");
  devices_["thermostat"] = thermo;

  // Create security camera in the Security subnet
  auto secIP = subnetManager_.allocateIP("Security");
  auto cam = std::make_shared<SecurityCamera>(
      "cam-1", "Front Door Camera", secIP,
      arpTable_.arpRequest(secIP), "Security");
  devices_["security"] = cam;

  Logger::getInstance().info(
      "Devices initialized: " +
      std::to_string(devices_.size()) + " devices");
}

bool Server::initSocket() {
#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    Logger::getInstance().error("WSAStartup failed");
    return false;
  }
#endif

  serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket_ == INVALID_SOCK) {
    Logger::getInstance().error("Failed to create socket");
    return false;
  }

  // Allow port reuse
  int opt = 1;
#ifdef _WIN32
  setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR,
             reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
  setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt,
             sizeof(opt));
#endif

  struct sockaddr_in serverAddr {};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(port_);

  if (bind(serverSocket_,
           reinterpret_cast<struct sockaddr*>(&serverAddr),
           sizeof(serverAddr)) < 0) {
    Logger::getInstance().error("Bind failed");
    cleanupSocket();
    return false;
  }

  if (listen(serverSocket_, NetworkConfig::MAX_CLIENTS) < 0) {
    Logger::getInstance().error("Listen failed");
    cleanupSocket();
    return false;
  }

  Logger::getInstance().info("Socket initialized on port " +
                             std::to_string(port_));
  return true;
}

void Server::acceptClients() {
  ThreadPool pool(4);

  while (running_) {
    struct sockaddr_in clientAddr {};
#ifdef _WIN32
    int addrLen = sizeof(clientAddr);
#else
    socklen_t addrLen = sizeof(clientAddr);
#endif

    SocketType clientSocket =
        accept(serverSocket_,
               reinterpret_cast<struct sockaddr*>(&clientAddr),
               &addrLen);

    if (clientSocket == INVALID_SOCK) {
      if (running_) {
        Logger::getInstance().error("Accept failed");
      }
      continue;
    }

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr,
              INET_ADDRSTRLEN);
    std::string clientIP(ipStr);
    clientIP += ":" + std::to_string(ntohs(clientAddr.sin_port));

    pool.enqueue([clientSocket, clientIP, this]() {
      ClientHandler handler(clientSocket, clientIP, this);
      handler.handle();
    });
  }
}

void Server::cleanupSocket() {
  if (serverSocket_ != INVALID_SOCK) {
#ifdef _WIN32
    closesocket(serverSocket_);
    WSACleanup();
#else
    close(serverSocket_);
#endif
    serverSocket_ = INVALID_SOCK;
  }
}

std::string Server::processCommand(const std::string& rawRequest) {
  // Check for PING command (not HTTP-formatted)
  if (rawRequest.substr(0, 4) == "PING" ||
      rawRequest.substr(0, 4) == "ping") {
    size_t spacePos = rawRequest.find(' ');
    if (spacePos != std::string::npos) {
      std::string targetIP = rawRequest.substr(spacePos + 1);
      // Trim whitespace
      while (!targetIP.empty() &&
             (targetIP.back() == '\r' || targetIP.back() == '\n' ||
              targetIP.back() == ' ')) {
        targetIP.pop_back();
      }
      return handlePingCommand(targetIP);
    }
    return HttpProtocol::errorResponse("Usage: PING <ip>")
        .serialize();
  }

  // Check for ARP command
  if (rawRequest.substr(0, 3) == "ARP" ||
      rawRequest.substr(0, 3) == "arp") {
    size_t spacePos = rawRequest.find(' ');
    if (spacePos != std::string::npos) {
      std::string targetIP = rawRequest.substr(spacePos + 1);
      while (!targetIP.empty() &&
             (targetIP.back() == '\r' || targetIP.back() == '\n' ||
              targetIP.back() == ' ')) {
        targetIP.pop_back();
      }
      return handleArpCommand(targetIP);
    }
    return HttpProtocol::errorResponse("Usage: ARP <ip>")
        .serialize();
  }

  // Parse as HTTP request
  HttpRequest req = HttpProtocol::parseRequest(rawRequest);
  auto segments = req.getPathSegments();

  if (segments.empty()) {
    return HttpProtocol::notFoundResponse("No command specified")
        .serialize();
  }

  const std::string& resource = segments[0];

  if (resource == "light") {
    return handleLightCommand(segments);
  } else if (resource == "thermostat") {
    return handleThermostatCommand(segments);
  } else if (resource == "security") {
    return handleSecurityCommand(segments);
  } else if (resource == "devices") {
    return handleDeviceListCommand();
  }

  return HttpProtocol::notFoundResponse(
             "Unknown resource: " + resource)
      .serialize();
}

std::string Server::handleLightCommand(
    const std::vector<std::string>& segments) {
  std::lock_guard<std::mutex> lock(devicesMutex_);
  auto it = devices_.find("light");
  if (it == devices_.end()) {
    return HttpProtocol::errorResponse("Light not found")
        .serialize();
  }

  auto light = std::dynamic_pointer_cast<Light>(it->second);
  if (!light) {
    return HttpProtocol::errorResponse("Device cast failed")
        .serialize();
  }

  if (segments.size() < 2) {
    return HttpProtocol::notFoundResponse("Usage: /light/<action>")
        .serialize();
  }

  const std::string& action = segments[1];

  if (action == "status") {
    return HttpProtocol::okResponse(light->getDetailedStatus())
        .serialize();
  } else if (action == "on") {
    light->turnOn();
    return HttpProtocol::okResponse("Light turned ON").serialize();
  } else if (action == "off") {
    light->turnOff();
    return HttpProtocol::okResponse("Light turned OFF").serialize();
  } else if (action == "brightness" && segments.size() >= 3) {
    try {
      int level = std::stoi(segments[2]);
      light->setBrightness(level);
      return HttpProtocol::okResponse(
                 "Brightness set to " + std::to_string(level))
          .serialize();
    } catch (...) {
      return HttpProtocol::errorResponse("Invalid brightness value")
          .serialize();
    }
  }

  return HttpProtocol::notFoundResponse(
             "Unknown light action: " + action)
      .serialize();
}

std::string Server::handleThermostatCommand(
    const std::vector<std::string>& segments) {
  std::lock_guard<std::mutex> lock(devicesMutex_);
  auto it = devices_.find("thermostat");
  if (it == devices_.end()) {
    return HttpProtocol::errorResponse("Thermostat not found")
        .serialize();
  }

  auto thermo =
      std::dynamic_pointer_cast<Thermostat>(it->second);
  if (!thermo) {
    return HttpProtocol::errorResponse("Device cast failed")
        .serialize();
  }

  if (segments.size() < 2) {
    return HttpProtocol::notFoundResponse(
               "Usage: /thermostat/<action>")
        .serialize();
  }

  const std::string& action = segments[1];

  if (action == "status") {
    return HttpProtocol::okResponse(thermo->getDetailedStatus())
        .serialize();
  } else if (action == "set" && segments.size() >= 3) {
    try {
      double temp = std::stod(segments[2]);
      thermo->setTargetTemperature(temp);
      return HttpProtocol::okResponse(
                 "Temperature set to " + segments[2] + " C")
          .serialize();
    } catch (...) {
      return HttpProtocol::errorResponse(
                 "Invalid temperature value")
          .serialize();
    }
  }

  return HttpProtocol::notFoundResponse(
             "Unknown thermostat action: " + action)
      .serialize();
}

std::string Server::handleSecurityCommand(
    const std::vector<std::string>& segments) {
  std::lock_guard<std::mutex> lock(devicesMutex_);
  auto it = devices_.find("security");
  if (it == devices_.end()) {
    return HttpProtocol::errorResponse("Security camera not found")
        .serialize();
  }

  auto cam =
      std::dynamic_pointer_cast<SecurityCamera>(it->second);
  if (!cam) {
    return HttpProtocol::errorResponse("Device cast failed")
        .serialize();
  }

  if (segments.size() < 2) {
    return HttpProtocol::notFoundResponse(
               "Usage: /security/<action>")
        .serialize();
  }

  const std::string& action = segments[1];

  if (action == "status") {
    return HttpProtocol::okResponse(cam->getDetailedStatus())
        .serialize();
  } else if (action == "arm") {
    cam->arm();
    return HttpProtocol::okResponse("Security camera armed")
        .serialize();
  } else if (action == "disarm") {
    cam->disarm();
    return HttpProtocol::okResponse("Security camera disarmed")
        .serialize();
  }

  return HttpProtocol::notFoundResponse(
             "Unknown security action: " + action)
      .serialize();
}

std::string Server::handleDeviceListCommand() {
  std::lock_guard<std::mutex> lock(devicesMutex_);
  std::ostringstream oss;
  oss << "=== Device List ===\n";
  for (const auto& [key, device] : devices_) {
    oss << device->getName() << " [" << device->getId() << "] - "
        << device->getTypeString() << " - "
        << device->getStatus() << " - IP: "
        << device->getIpAddress() << " - Subnet: "
        << device->getSubnet() << "\n";
  }
  oss << "Total devices: " << devices_.size();
  return HttpProtocol::okResponse(oss.str()).serialize();
}

std::string Server::handlePingCommand(
    const std::string& targetIP) {
  Logger::getInstance().info("PING request for " + targetIP);

  // Check routing
  auto route = routingTable_.lookupRoute(targetIP);
  std::string routeInfo = "Route: via " + route.gateway +
                          " on " + route.interface_ + "\n";

  std::string pingResult = ICMPHandler::ping(targetIP);
  return routeInfo + pingResult;
}

std::string Server::handleArpCommand(
    const std::string& targetIP) {
  Logger::getInstance().info("ARP request for " + targetIP);

  std::string mac = arpTable_.arpRequest(targetIP);
  std::string subnet = subnetManager_.findSubnet(targetIP);

  std::ostringstream oss;
  oss << "ARP Resolution:\n"
      << "  IP: " << targetIP << "\n"
      << "  MAC: " << mac << "\n"
      << "  Subnet: " << subnet;
  return oss.str();
}