#include "devices/Light.h"

#include <sstream>

Light::Light(const std::string& id, const std::string& name,
             const std::string& ipAddress,
             const std::string& macAddress,
             const std::string& subnet)
    : Device(id, name, DeviceType::LIGHT, ipAddress, macAddress,
             subnet),
      brightness_(100),
      color_("white") {}

void Light::setBrightness(int level) {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  if (level < 0) level = 0;
  if (level > 100) level = 100;
  brightness_ = level;
}

int Light::getBrightness() const {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  return brightness_;
}

void Light::setColor(const std::string& color) {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  color_ = color;
}

std::string Light::getColor() const {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  return color_;
}

std::string Light::getDetailedStatus() const {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  std::ostringstream oss;
  oss << "Device: " << name_ << " [" << id_ << "]\n"
      << "  Type: Light\n"
      << "  Power: " << (powerOn_ ? "ON" : "OFF") << "\n"
      << "  Brightness: " << brightness_ << "%\n"
      << "  Color: " << color_ << "\n"
      << "  IP: " << ipAddress_ << "\n"
      << "  MAC: " << macAddress_ << "\n"
      << "  Subnet: " << subnet_;
  return oss.str();
}