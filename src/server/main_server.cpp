#include <iostream>

#include "network/NetworkConfig.h"
#include "server/Server.h"

int main() {
  std::cout << "=== Smart Home Network Server ===" << std::endl;

  Server server(NetworkConfig::SERVER_PORT);
  server.start();

  return 0;
}