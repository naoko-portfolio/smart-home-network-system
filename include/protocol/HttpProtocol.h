#ifndef HTTP_PROTOCOL_H
#define HTTP_PROTOCOL_H

#include <map>
#include <string>
#include <vector>

// Represents a parsed HTTP-like request
struct HttpRequest {
  std::string method;   // GET, POST, etc.
  std::string path;     // e.g., /light/on
  std::string version;  // e.g., SHCP/1.0
  std::map<std::string, std::string> headers;
  std::string body;

  // Parse path segments: /light/on -> ["light", "on"]
  std::vector<std::string> getPathSegments() const;
};

// Represents an HTTP-like response
struct HttpResponse {
  int statusCode;
  std::string statusMessage;
  std::string version;
  std::map<std::string, std::string> headers;
  std::string body;

  // Serialize to string for sending over socket
  std::string serialize() const;
};

// Protocol parser and builder
class HttpProtocol {
 public:
  HttpProtocol() = default;
  ~HttpProtocol() = default;

  // Parse a raw request string into HttpRequest
  static HttpRequest parseRequest(const std::string& raw);

  // Build a raw request string from components
  static std::string buildRequest(const std::string& method,
                                  const std::string& path,
                                  const std::string& body = "");

  // Build common responses
  static HttpResponse buildResponse(int statusCode,
                                    const std::string& body);
  static HttpResponse okResponse(const std::string& body);
  static HttpResponse notFoundResponse(const std::string& msg);
  static HttpResponse errorResponse(const std::string& msg);
};

#endif  // HTTP_PROTOCOL_H