#include "protocol/ICMPHandler.h"

#include <chrono>
#include <random>
#include <sstream>
#include <thread>

#include "network/NetworkConfig.h"
#include "utils/Logger.h"

std::string ICMPHandler::ping(const std::string& targetIP,
                              int count, int timeoutMs) {
  std::ostringstream oss;
  oss << "PING " << targetIP << " with " << count
      << " packets:\n";

  int received = 0;
  int totalLatency = 0;

  for (int i = 1; i <= count; i++) {
    int latency = simulateLatency();

    // Simulate timeout for unreachable IPs
    // Consider IPs outside our network as unreachable
    bool reachable = NetworkConfig::isInSubnet(
        targetIP, "192.168.1.0", 24);

    if (reachable && latency < timeoutMs) {
      oss << "Reply from " << targetIP << ": seq=" << i
          << " ttl=64 time=" << latency << "ms\n";
      received++;
      totalLatency += latency;
    } else {
      oss << "Request timed out for seq=" << i << "\n";
    }

    // Small delay between pings
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  int lost = count - received;
  double lossPercent =
      (count > 0) ? (static_cast<double>(lost) / count * 100) : 0;
  double avgLatency =
      (received > 0) ? (static_cast<double>(totalLatency) / received)
                     : 0;

  oss << "\n--- " << targetIP << " ping statistics ---\n"
      << count << " packets transmitted, " << received
      << " received, " << lossPercent << "% loss\n"
      << "avg latency: " << avgLatency << "ms\n";

  return oss.str();
}

std::string ICMPHandler::echoReply(const std::string& sourceIP,
                                   int sequenceNum, int ttl) {
  int latency = simulateLatency();
  std::ostringstream oss;
  oss << "Echo reply from " << sourceIP << ": seq=" << sequenceNum
      << " ttl=" << ttl << " time=" << latency << "ms";
  return oss.str();
}

std::string ICMPHandler::destinationUnreachable(
    const std::string& targetIP) {
  return "ICMP: Destination " + targetIP +
         " unreachable (host not found)";
}

int ICMPHandler::simulateLatency() {
  // Random latency between 1-50 ms
  static thread_local std::mt19937 gen(std::random_device{}());
  std::uniform_int_distribution<int> dist(1, 50);
  return dist(gen);
}