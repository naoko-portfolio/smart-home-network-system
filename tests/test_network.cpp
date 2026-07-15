#include <cassert>
#include <iostream>
#include <string>

#include "network/ARPTable.h"
#include "network/NetworkConfig.h"
#include "network/RoutingTable.h"
#include "network/SubnetManager.h"

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

void testNetworkConfig() {
  std::cout << "\n=== NetworkConfig Tests ===" << std::endl;

  // IP conversion
  uint32_t ip = NetworkConfig::ipToUint32("192.168.1.0");
  TEST("ipToUint32 192.168.1.0", ip == 0xC0A80100);

  std::string ipStr = NetworkConfig::uint32ToIp(0xC0A80100);
  TEST("uint32ToIp 0xC0A80100", ipStr == "192.168.1.0");

  // Round-trip
  std::string original = "10.0.0.1";
  TEST("IP round-trip",
       NetworkConfig::uint32ToIp(
           NetworkConfig::ipToUint32(original)) == original);

  // CIDR to mask
  TEST("cidrToMask /24",
       NetworkConfig::cidrToMask(24) == "255.255.255.0");
  TEST("cidrToMask /26",
       NetworkConfig::cidrToMask(26) == "255.255.255.192");
  TEST("cidrToMask /27",
       NetworkConfig::cidrToMask(27) == "255.255.255.224");
  TEST("cidrToMask /28",
       NetworkConfig::cidrToMask(28) == "255.255.255.240");
  TEST("cidrToMask /32",
       NetworkConfig::cidrToMask(32) == "255.255.255.255");
  TEST("cidrToMask /0",
       NetworkConfig::cidrToMask(0) == "0.0.0.0");

  // isInSubnet
  TEST("192.168.1.5 in 192.168.1.0/26",
       NetworkConfig::isInSubnet("192.168.1.5", "192.168.1.0", 26));
  TEST("192.168.1.62 in 192.168.1.0/26",
       NetworkConfig::isInSubnet("192.168.1.62", "192.168.1.0", 26));
  TEST("192.168.1.64 NOT in 192.168.1.0/26",
       !NetworkConfig::isInSubnet("192.168.1.64", "192.168.1.0", 26));
  TEST("192.168.1.65 in 192.168.1.64/27",
       NetworkConfig::isInSubnet("192.168.1.65", "192.168.1.64", 27));
  TEST("10.0.0.1 NOT in 192.168.1.0/24",
       !NetworkConfig::isInSubnet("10.0.0.1", "192.168.1.0", 24));

  // Subnet configurations
  auto subnets = NetworkConfig::getSubnetConfigurations();
  TEST("4 subnets configured", subnets.size() == 4);
  TEST("First subnet is Lighting", subnets[0].name == "Lighting");
  TEST("Second subnet is Thermostat",
       subnets[1].name == "Thermostat");
  TEST("Third subnet is Security", subnets[2].name == "Security");
  TEST("Fourth subnet is Management",
       subnets[3].name == "Management");
}

void testSubnetManager() {
  std::cout << "\n=== SubnetManager Tests ===" << std::endl;
  SubnetManager mgr;
  mgr.initializeSubnets();

  // Allocate IPs
  std::string ip1 = mgr.allocateIP("Lighting");
  TEST("First Lighting IP is 192.168.1.2", ip1 == "192.168.1.2");

  std::string ip2 = mgr.allocateIP("Lighting");
  TEST("Second Lighting IP is 192.168.1.3", ip2 == "192.168.1.3");

  std::string ip3 = mgr.allocateIP("Thermostat");
  TEST("First Thermostat IP is 192.168.1.66",
       ip3 == "192.168.1.66");

  std::string ip4 = mgr.allocateIP("Security");
  TEST("First Security IP is 192.168.1.98",
       ip4 == "192.168.1.98");

  // Find subnet
  TEST("Find subnet for 192.168.1.2",
       mgr.findSubnet("192.168.1.2") == "Lighting");
  TEST("Find subnet for 192.168.1.66",
       mgr.findSubnet("192.168.1.66") == "Thermostat");
  TEST("Find subnet for 192.168.1.98",
       mgr.findSubnet("192.168.1.98") == "Security");
  TEST("Find subnet for unknown IP",
       mgr.findSubnet("10.0.0.1") == "Unknown");

  // Get subnet info
  auto info = mgr.getSubnetInfo("Lighting");
  TEST("Lighting network address",
       info.networkAddress == "192.168.1.0");
  TEST("Lighting CIDR", info.cidr == 26);
  TEST("Lighting max hosts", info.maxHosts == 62);

  // Release IP
  mgr.releaseIP("Lighting", ip1);
  TEST("Release IP without crash", true);

  // Invalid subnet throws
  bool threw = false;
  try {
    mgr.allocateIP("NonExistent");
  } catch (const std::runtime_error&) {
    threw = true;
  }
  TEST("Allocate from invalid subnet throws", threw);
}

void testRoutingTable() {
  std::cout << "\n=== RoutingTable Tests ===" << std::endl;
  RoutingTable rt;

  // Add routes
  rt.addRoute("192.168.1.0", 26, "192.168.1.1", "Lighting");
  rt.addRoute("192.168.1.64", 27, "192.168.1.65", "Thermostat");
  rt.addRoute("192.168.1.96", 27, "192.168.1.97", "Security");
  rt.addRoute("192.168.1.128", 28, "192.168.1.129", "Management");
  rt.setDefaultGateway("192.168.1.1", "Management");

  // Lookup routes - longest prefix match
  auto route1 = rt.lookupRoute("192.168.1.10");
  TEST("Route for 192.168.1.10 is Lighting",
       route1.interface_ == "Lighting");

  auto route2 = rt.lookupRoute("192.168.1.70");
  TEST("Route for 192.168.1.70 is Thermostat",
       route2.interface_ == "Thermostat");

  auto route3 = rt.lookupRoute("192.168.1.100");
  TEST("Route for 192.168.1.100 is Security",
       route3.interface_ == "Security");

  auto route4 = rt.lookupRoute("192.168.1.130");
  TEST("Route for 192.168.1.130 is Management",
       route4.interface_ == "Management");

  // Default route for unknown destination
  auto route5 = rt.lookupRoute("10.0.0.1");
  TEST("Unknown dest uses default gateway",
       route5.gateway == "192.168.1.1");

  // Remove route
  rt.removeRoute("192.168.1.96", 27);
  auto route6 = rt.lookupRoute("192.168.1.100");
  TEST("After removing Security route, falls to default",
       route6.gateway == "192.168.1.1");
}

void testARPTable() {
  std::cout << "\n=== ARPTable Tests ===" << std::endl;
  ARPTable arp;

  // Initially empty
  TEST("Resolve unknown IP returns empty",
       arp.resolve("192.168.1.5") == "");

  // Manual entry
  arp.addEntry("192.168.1.5", "AA:BB:CC:DD:EE:FF", true);
  TEST("Resolve after addEntry",
       arp.resolve("192.168.1.5") == "AA:BB:CC:DD:EE:FF");

  // ARP request generates MAC
  std::string mac = arp.arpRequest("192.168.1.10");
  TEST("ARP request returns non-empty MAC", !mac.empty());
  TEST("ARP request starts with AA:", mac.substr(0, 3) == "AA:");

  // Cached
  std::string mac2 = arp.arpRequest("192.168.1.10");
  TEST("Cached ARP returns same MAC", mac == mac2);

  // Different IP gets different MAC
  std::string mac3 = arp.arpRequest("192.168.1.20");
  TEST("Different IP gets different MAC", mac3 != mac);

  // Remove entry
  arp.removeEntry("192.168.1.5");
  TEST("After removeEntry, resolve returns empty",
       arp.resolve("192.168.1.5") == "");

  // Flush expired - static entries survive, dynamic with age check
  arp.arpRequest("192.168.1.30");
  // Entry just created has age ~0, so maxAge=-1 flushes everything
  arp.flushExpired(-1);
  TEST("After flushExpired(-1), dynamic entry removed",
       arp.resolve("192.168.1.30") == "");
}

int main() {
  std::cout << "========== Network Tests ==========" << std::endl;

  testNetworkConfig();
  testSubnetManager();
  testRoutingTable();
  testARPTable();

  std::cout << "\n========== Results ==========" << std::endl;
  std::cout << "Passed: " << passed << std::endl;
  std::cout << "Failed: " << failed << std::endl;

  return (failed == 0) ? 0 : 1;
}
