#include "devices/Thermostat.h"

#include <sstream>

Thermostat::Thermostat(const std::string& id,
                       const std::string& name,
                       const std::string& ipAddress,
                       const std::string& macAddress,
                       const std::string& subnet)
    : Device(id, name, DeviceType::THERMOSTAT, ipAddress,
             macAddress, subnet),
      targetTemperature_(22.0),
      currentTemperature_(20.0),
      mode_(Mode::AUTO) {}

void Thermostat::setTargetTemperature(double temp) {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  if (temp < 10.0) temp = 10.0;
  if (temp > 35.0) temp = 35.0;
  targetTemperature_ = temp;
}

double Thermostat::getTargetTemperature() const {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  return targetTemperature_;
}

double Thermostat::getCurrentTemperature() const {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  return currentTemperature_;
}

void Thermostat::setMode(Mode mode) {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  mode_ = mode;
}

Thermostat::Mode Thermostat::getMode() const {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  return mode_;
}

std::string Thermostat::getModeString() const {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  switch (mode_) {
    case Mode::OFF:
      return "OFF";
    case Mode::HEAT:
      return "HEAT";
    case Mode::COOL:
      return "COOL";
    case Mode::AUTO:
      return "AUTO";
    default:
      return "UNKNOWN";
  }
}

std::string Thermostat::getDetailedStatus() const {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  std::string modeStr;
  switch (mode_) {
    case Mode::OFF:
      modeStr = "OFF";
      break;
    case Mode::HEAT:
      modeStr = "HEAT";
      break;
    case Mode::COOL:
      modeStr = "COOL";
      break;
    case Mode::AUTO:
      modeStr = "AUTO";
      break;
  }

  std::ostringstream oss;
  oss << "Device: " << name_ << " [" << id_ << "]\n"
      << "  Type: Thermostat\n"
      << "  Power: " << (powerOn_ ? "ON" : "OFF") << "\n"
      << "  Mode: " << modeStr << "\n"
      << "  Target Temp: " << targetTemperature_ << " C\n"
      << "  Current Temp: " << currentTemperature_ << " C\n"
      << "  IP: " << ipAddress_ << "\n"
      << "  MAC: " << macAddress_ << "\n"
      << "  Subnet: " << subnet_;
  return oss.str();
}