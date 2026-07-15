#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "devices/Light.h"
#include "devices/SecurityCamera.h"
#include "devices/Thermostat.h"
#include "network/ARPTable.h"
#include "network/NetworkConfig.h"
#include "network/RoutingTable.h"
#include "network/SubnetManager.h"
#include "protocol/HttpProtocol.h"
#include "server/Server.h"

static int passed = 0;
static int failed = 0;

#define TEST(name, expr)                                       \
  do {                                                         \
    if (expr) {                                                \
      std::cout << "  PASS: " << name << std::endl;            \
      passed++;                                                \
    } else {                                                   \
      std::cout << "  FAIL: " << name << std::endl;            \
      failed++;                                                \
    }                                                          \
  } while (0)

// Test processCommand without starting the socket/network
// We create a Server, initialize devices, and call processCommand directly
class ServerTester {
 public:
  ServerTester() : server_(0) {
    // Initialize network and devices, but not the socket
    server_.initializeNetwork();
    server_.initializeDevices();
  }

  std::string processCommand(const std::string& cmd) {
    return server_.processCommand(cmd);
  }

 private:
  Server server_;
};

void testLightCommands() {
  std::cout << "\n=== Light Command Tests ===" << std::endl;
  ServerTester tester;

  // Light status
  std::string resp = tester.processCommand(
      "GET /light/status SHCP/1.0\r\n\r\n");
  TEST("Light status returns 200",
       resp.find("200 OK") != std::string::npos);
  TEST("Light status contains device info",
       resp.find("Light") != std::string::npos);

  // Light on
  resp = tester.processCommand("GET /light/on SHCP/1.0\r\n\r\n");
  TEST("Light on returns 200",
       resp.find("200 OK") != std::string::npos);
  TEST("Light on confirmation",
       resp.find("Light turned ON") != std::string::npos);

  // Light off
  resp = tester.processCommand("GET /light/off SHCP/1.0\r\n\r\n");
  TEST("Light off returns 200",
       resp.find("200 OK") != std::string::npos);
  TEST("Light off confirmation",
       resp.find("Light turned OFF") != std::string::npos);

  // Light brightness
  resp = tester.processCommand(
      "GET /light/brightness/75 SHCP/1.0\r\n\r\n");
  TEST("Brightness returns 200",
       resp.find("200 OK") != std::string::npos);
  TEST("Brightness set confirmation",
       resp.find("Brightness set to 75") != std::string::npos);

  // Verify brightness persisted
  resp = tester.processCommand(
      "GET /light/status SHCP/1.0\r\n\r\n");
  TEST("Brightness persisted in status",
       resp.find("75%") != std::string::npos);
}

void testThermostatCommands() {
  std::cout << "\n=== Thermostat Command Tests ===" << std::endl;
  ServerTester tester;

  // Status
  std::string resp = tester.processCommand(
      "GET /thermostat/status SHCP/1.0\r\n\r\n");
  TEST("Thermostat status returns 200",
       resp.find("200 OK") != std::string::npos);
  TEST("Thermostat status contains info",
       resp.find("Thermostat") != std::string::npos);

  // Set temperature
  resp = tester.processCommand(
      "GET /thermostat/set/25 SHCP/1.0\r\n\r\n");
  TEST("Set temp returns 200",
       resp.find("200 OK") != std::string::npos);
  TEST("Set temp confirmation",
       resp.find("Temperature set to 25") != std::string::npos);

  // Verify temp persisted
  resp = tester.processCommand(
      "GET /thermostat/status SHCP/1.0\r\n\r\n");
  TEST("Temperature persisted",
       resp.find("25") != std::string::npos);
}

void testSecurityCommands() {
  std::cout << "\n=== Security Command Tests ===" << std::endl;
  ServerTester tester;

  // Status
  std::string resp = tester.processCommand(
      "GET /security/status SHCP/1.0\r\n\r\n");
  TEST("Security status returns 200",
       resp.find("200 OK") != std::string::npos);

  // Arm
  resp = tester.processCommand(
      "GET /security/arm SHCP/1.0\r\n\r\n");
  TEST("Arm returns 200",
       resp.find("200 OK") != std::string::npos);
  TEST("Arm confirmation",
       resp.find("armed") != std::string::npos);

  // Verify armed in status
  resp = tester.processCommand(
      "GET /security/status SHCP/1.0\r\n\r\n");
  TEST("Armed shows in status",
       resp.find("YES") != std::string::npos);

  // Disarm
  resp = tester.processCommand(
      "GET /security/disarm SHCP/1.0\r\n\r\n");
  TEST("Disarm returns 200",
       resp.find("200 OK") != std::string::npos);
  TEST("Disarm confirmation",
       resp.find("disarmed") != std::string::npos);
}

void testDeviceListCommand() {
  std::cout << "\n=== Device List Command Tests ===" << std::endl;
  ServerTester tester;

  std::string resp = tester.processCommand(
      "GET /devices/list SHCP/1.0\r\n\r\n");
  TEST("Device list returns 200",
       resp.find("200 OK") != std::string::npos);
  TEST("Device list contains light",
       resp.find("Light") != std::string::npos);
  TEST("Device list contains thermostat",
       resp.find("Thermostat") != std::string::npos);
  TEST("Device list contains security",
       resp.find("Security") != std::string::npos ||
       resp.find("Camera") != std::string::npos);
  TEST("Device list shows count",
       resp.find("Total devices:") != std::string::npos);
}

void testInvalidCommands() {
  std::cout << "\n=== Invalid Command Tests ===" << std::endl;
  ServerTester tester;

  // Unknown resource
  std::string resp = tester.processCommand(
      "GET /fridge/status SHCP/1.0\r\n\r\n");
  TEST("Unknown resource returns 404",
       resp.find("404") != std::string::npos);

  // Unknown light action
  resp = tester.processCommand(
      "GET /light/explode SHCP/1.0\r\n\r\n");
  TEST("Unknown light action returns 404",
       resp.find("404") != std::string::npos);

  // Invalid brightness value
  resp = tester.processCommand(
      "GET /light/brightness/abc SHCP/1.0\r\n\r\n");
  TEST("Invalid brightness returns 500",
       resp.find("500") != std::string::npos);

  // Invalid temperature
  resp = tester.processCommand(
      "GET /thermostat/set/abc SHCP/1.0\r\n\r\n");
  TEST("Invalid temperature returns 500",
       resp.find("500") != std::string::npos);

  // Empty path
  resp = tester.processCommand("GET / SHCP/1.0\r\n\r\n");
  TEST("Empty path returns 404",
       resp.find("404") != std::string::npos);
}

void testPingCommand() {
  std::cout << "\n=== PING Command Tests ===" << std::endl;
  ServerTester tester;

  // PING in-network IP
  std::string resp =
      tester.processCommand("PING 192.168.1.5\r\n");
  TEST("PING returns route info",
       resp.find("Route:") != std::string::npos);
  TEST("PING returns statistics",
       resp.find("ping statistics") != std::string::npos);

  // ARP command
  resp = tester.processCommand("ARP 192.168.1.10\r\n");
  TEST("ARP returns resolution info",
       resp.find("ARP Resolution") != std::string::npos);
  TEST("ARP returns MAC",
       resp.find("MAC:") != std::string::npos);
  TEST("ARP returns subnet info",
       resp.find("Subnet:") != std::string::npos);
}

int main() {
  std::cout << "========== Server Tests ==========" << std::endl;

  testLightCommands();
  testThermostatCommands();
  testSecurityCommands();
  testDeviceListCommand();
  testInvalidCommands();
  testPingCommand();

  std::cout << "\n========== Results ==========" << std::endl;
  std::cout << "Passed: " << passed << std::endl;
  std::cout << "Failed: " << failed << std::endl;

  return (failed == 0) ? 0 : 1;
}
