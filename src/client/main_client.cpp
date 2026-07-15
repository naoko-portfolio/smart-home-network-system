#include <iostream>

#include "client/Client.h"
#include "network/NetworkConfig.h"

int main() {
  std::cout << "=== Smart Home Network Client ===" << std::endl;

  Client client(NetworkConfig::SERVER_IP,
                NetworkConfig::SERVER_PORT);

  if (!client.connectToServer()) {
    std::cerr << "Failed to connect to server." << std::endl;
    return 1;
  }

  client.interactiveMode();
  client.disconnect();

  return 0;
}