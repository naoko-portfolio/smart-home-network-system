#include <cassert>
#include <iostream>
#include <string>

#include "protocol/HttpProtocol.h"
#include "protocol/ICMPHandler.h"
#include "network/NetworkConfig.h"

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

void testParseRequest() {
  std::cout << "\n=== Parse Request Tests ===" << std::endl;

  // Standard request
  std::string raw = "GET /light/on SHCP/1.0\r\nHost: SmartHome\r\n\r\n";
  HttpRequest req = HttpProtocol::parseRequest(raw);
  TEST("Method is GET", req.method == "GET");
  TEST("Path is /light/on", req.path == "/light/on");
  TEST("Version is SHCP/1.0", req.version == "SHCP/1.0");
  TEST("Host header parsed",
       req.headers.count("Host") && req.headers.at("Host") == "SmartHome");

  // Path segments
  auto segs = req.getPathSegments();
  TEST("2 path segments", segs.size() == 2);
  TEST("First segment is light", segs[0] == "light");
  TEST("Second segment is on", segs[1] == "on");

  // Request with 3 segments
  raw = "GET /light/brightness/50 SHCP/1.0\r\n\r\n";
  req = HttpProtocol::parseRequest(raw);
  segs = req.getPathSegments();
  TEST("3 path segments", segs.size() == 3);
  TEST("Third segment is 50", segs[2] == "50");

  // Request with no version defaults
  raw = "GET /devices/list\r\n\r\n";
  req = HttpProtocol::parseRequest(raw);
  TEST("Default version is SHCP/1.0", req.version == "SHCP/1.0");
  TEST("Path is /devices/list", req.path == "/devices/list");

  // Empty request
  raw = "";
  req = HttpProtocol::parseRequest(raw);
  TEST("Empty request has empty method", req.method.empty());
}

void testBuildRequest() {
  std::cout << "\n=== Build Request Tests ===" << std::endl;

  std::string req = HttpProtocol::buildRequest("GET", "/light/status");
  TEST("Built request contains GET",
       req.find("GET") != std::string::npos);
  TEST("Built request contains path",
       req.find("/light/status") != std::string::npos);
  TEST("Built request contains SHCP/1.0",
       req.find("SHCP/1.0") != std::string::npos);
  TEST("Built request contains Host header",
       req.find("Host: SmartHome") != std::string::npos);

  // With body
  std::string reqBody =
      HttpProtocol::buildRequest("POST", "/light/set", "brightness=50");
  TEST("Request with body contains Content-Length",
       reqBody.find("Content-Length") != std::string::npos);
  TEST("Request with body contains body text",
       reqBody.find("brightness=50") != std::string::npos);
}

void testBuildResponse() {
  std::cout << "\n=== Build Response Tests ===" << std::endl;

  auto resp = HttpProtocol::okResponse("Light is ON");
  TEST("OK response code is 200", resp.statusCode == 200);
  TEST("OK response message", resp.statusMessage == "OK");
  TEST("OK response body", resp.body == "Light is ON");

  auto resp404 = HttpProtocol::notFoundResponse("Not found");
  TEST("404 response code", resp404.statusCode == 404);
  TEST("404 response message", resp404.statusMessage == "Not Found");

  auto resp500 = HttpProtocol::errorResponse("Server error");
  TEST("500 response code", resp500.statusCode == 500);
  TEST("500 response message",
       resp500.statusMessage == "Internal Server Error");
}

void testSerializeResponse() {
  std::cout << "\n=== Serialize Response Tests ===" << std::endl;

  auto resp = HttpProtocol::okResponse("Hello");
  std::string serialized = resp.serialize();

  TEST("Serialized contains SHCP/1.0",
       serialized.find("SHCP/1.0") != std::string::npos);
  TEST("Serialized contains 200 OK",
       serialized.find("200 OK") != std::string::npos);
  TEST("Serialized contains Content-Length",
       serialized.find("Content-Length: 5") != std::string::npos);
  TEST("Serialized contains body",
       serialized.find("Hello") != std::string::npos);
  TEST("Serialized contains Server header",
       serialized.find("SmartHomeServer/1.0") != std::string::npos);

  // Round-trip: build then parse the serialized response
  // (response format is similar enough to request for basic parsing)
}

void testRoundTrip() {
  std::cout << "\n=== Round-Trip Tests ===" << std::endl;

  // Build a request, parse it back
  std::string raw =
      HttpProtocol::buildRequest("GET", "/thermostat/set/25");
  HttpRequest req = HttpProtocol::parseRequest(raw);
  TEST("Round-trip method", req.method == "GET");
  TEST("Round-trip path", req.path == "/thermostat/set/25");

  auto segs = req.getPathSegments();
  TEST("Round-trip segments count", segs.size() == 3);
  TEST("Round-trip first segment", segs[0] == "thermostat");
  TEST("Round-trip second segment", segs[1] == "set");
  TEST("Round-trip third segment", segs[2] == "25");
}

void testICMPHandler() {
  std::cout << "\n=== ICMP Handler Tests ===" << std::endl;

  // Ping a reachable IP (in 192.168.1.0/24)
  std::string result = ICMPHandler::ping("192.168.1.5", 2, 1000);
  TEST("Ping result contains target IP",
       result.find("192.168.1.5") != std::string::npos);
  TEST("Ping result contains statistics",
       result.find("ping statistics") != std::string::npos);
  TEST("Ping result contains packets transmitted",
       result.find("2 packets transmitted") != std::string::npos);

  // Ping an unreachable IP
  std::string unreachable = ICMPHandler::ping("10.0.0.1", 2, 1000);
  TEST("Unreachable ping shows timeout",
       unreachable.find("timed out") != std::string::npos);
  TEST("Unreachable ping shows 100% loss",
       unreachable.find("100%") != std::string::npos);

  // Echo reply
  std::string echo = ICMPHandler::echoReply("192.168.1.5", 1, 64);
  TEST("Echo reply contains IP",
       echo.find("192.168.1.5") != std::string::npos);
  TEST("Echo reply contains seq",
       echo.find("seq=1") != std::string::npos);

  // Destination unreachable
  std::string unreach =
      ICMPHandler::destinationUnreachable("10.0.0.1");
  TEST("Destination unreachable message",
       unreach.find("unreachable") != std::string::npos);
}

int main() {
  std::cout << "========== Protocol Tests ==========" << std::endl;

  testParseRequest();
  testBuildRequest();
  testBuildResponse();
  testSerializeResponse();
  testRoundTrip();
  testICMPHandler();

  std::cout << "\n========== Results ==========" << std::endl;
  std::cout << "Passed: " << passed << std::endl;
  std::cout << "Failed: " << failed << std::endl;

  return (failed == 0) ? 0 : 1;
}
