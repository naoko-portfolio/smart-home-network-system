#include "devices/Device.h"

Device::Device(const std::string& id, const std::string& name,
               DeviceType type, const std::string& ipAddress,
               const std::string& macAddress,
               const std::string& subnet)
    : id_(id),
      name_(name),
      type_(type),
      powerOn_(false),
      ipAddress_(ipAddress),
      macAddress_(macAddress),
      subnet_(subnet) {}

std::string Device::getId() const { return id_; }

std::string Device::getName() const { return name_; }

Device::DeviceType Device::getType() const { return type_; }

std::string Device::getTypeString() const {
  switch (type_) {
    case DeviceType::LIGHT:
      return "Light";
    case DeviceType::THERMOSTAT:
      return "Thermostat";
    case DeviceType::SECURITY_CAMERA:
      return "SecurityCamera";
    default:
      return "Unknown";
  }
}

bool Device::isOn() const {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  return powerOn_;
}

std::string Device::getIpAddress() const { return ipAddress_; }

std::string Device::getMacAddress() const { return macAddress_; }

std::string Device::getSubnet() const { return subnet_; }

void Device::turnOn() {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  powerOn_ = true;
}

void Device::turnOff() {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  powerOn_ = false;
}

std::string Device::getStatus() const {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  return powerOn_ ? "ON" : "OFF";
}