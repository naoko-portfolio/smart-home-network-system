#ifndef ICMP_HANDLER_H
#define ICMP_HANDLER_H

#include <string>

// Simulated ICMP for ping/echo functionality
class ICMPHandler {
 public:
  ICMPHandler() = default;
  ~ICMPHandler() = default;

  // Simulate a ping request to a target IP
  // Returns a formatted result string
  static std::string ping(const std::string& targetIP,
                          int count = 4, int timeoutMs = 1000);

  // Simulate an echo reply
  static std::string echoReply(const std::string& sourceIP,
                               int sequenceNum, int ttl = 64);

  // Simulate destination unreachable
  static std::string destinationUnreachable(
      const std::string& targetIP);

 private:
  // Simulate latency for realism
  static int simulateLatency();
};

#endif  // ICMP_HANDLER_H