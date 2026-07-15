#include "protocol/HttpProtocol.h"
#include <sstream>

std::vector<std::string> HttpRequest::getPathSegments() const {
  std::vector<std::string> segments;
  std::istringstream stream(path);
  std::string segment;
  while (std::getline(stream, segment, '/')) {
    if (!segment.empty()) {
      segments.push_back(segment);
    }
  }
  return segments;
}

std::string HttpResponse::serialize() const {
  std::ostringstream oss;
  oss << version << " " << statusCode << " " << statusMessage
      << "\r\n";
  for (const auto& [key, value] : headers) {
    oss << key << ": " << value << "\r\n";
  }
  oss << "Content-Length: " << body.size() << "\r\n";
  oss << "\r\n";
  oss << body;
  return oss.str();
}

HttpRequest HttpProtocol::parseRequest(const std::string& raw) {
  HttpRequest req;
  std::istringstream stream(raw);
  std::string line;

  // Parse request line: METHOD PATH VERSION
  if (std::getline(stream, line)) {
    // Remove trailing \r if present
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    std::istringstream reqLine(line);
    reqLine >> req.method >> req.path >> req.version;
    if (req.version.empty()) {
      req.version = "SHCP/1.0";
    }
  }

  // Parse headers
  while (std::getline(stream, line)) {
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    if (line.empty()) break;

    auto colonPos = line.find(':');
    if (colonPos != std::string::npos) {
      std::string key = line.substr(0, colonPos);
      std::string value = line.substr(colonPos + 1);
      // Trim leading spaces from value
      size_t start = value.find_first_not_of(' ');
      if (start != std::string::npos) {
        value = value.substr(start);
      }
      req.headers[key] = value;
    }
  }

  // Remaining is body
  std::ostringstream bodyStream;
  bodyStream << stream.rdbuf();
  req.body = bodyStream.str();

  return req;
}

std::string HttpProtocol::buildRequest(const std::string& method,
                                       const std::string& path,
                                       const std::string& body) {
  std::ostringstream oss;
  oss << method << " " << path << " SHCP/1.0\r\n";
  oss << "Host: SmartHome\r\n";
  if (!body.empty()) {
    oss << "Content-Length: " << body.size() << "\r\n";
  }
  oss << "\r\n";
  if (!body.empty()) {
    oss << body;
  }
  return oss.str();
}

HttpResponse HttpProtocol::buildResponse(int statusCode,
                                         const std::string& body) {
  HttpResponse resp;
  resp.statusCode = statusCode;
  resp.version = "SHCP/1.0";
  resp.body = body;

  switch (statusCode) {
    case 200:
      resp.statusMessage = "OK";
      break;
    case 404:
      resp.statusMessage = "Not Found";
      break;
    case 400:
      resp.statusMessage = "Bad Request";
      break;
    case 500:
      resp.statusMessage = "Internal Server Error";
      break;
    default:
      resp.statusMessage = "Unknown";
      break;
  }

  resp.headers["Server"] = "SmartHomeServer/1.0";
  resp.headers["Content-Type"] = "text/plain";
  return resp;
}

HttpResponse HttpProtocol::okResponse(const std::string& body) {
  return buildResponse(200, body);
}

HttpResponse HttpProtocol::notFoundResponse(
    const std::string& msg) {
  return buildResponse(404, msg);
}

HttpResponse HttpProtocol::errorResponse(const std::string& msg) {
  return buildResponse(500, msg);
}