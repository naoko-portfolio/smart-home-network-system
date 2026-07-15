#include "devices/SecurityCamera.h"

#include <ctime>
#include <sstream>

SecurityCamera::SecurityCamera(const std::string& id,
                               const std::string& name,
                               const std::string& ipAddress,
                               const std::string& macAddress,
                               const std::string& subnet)
    : Device(id, name, DeviceType::SECURITY_CAMERA, ipAddress,
             macAddress, subnet),
      armed_(false),
      motionDetected_(false),
      lastAlert_("None") {}

void SecurityCamera::arm() {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  armed_ = true;
  powerOn_ = true;
}

void SecurityCamera::disarm() {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  armed_ = false;
  motionDetected_ = false;
}

bool SecurityCamera::isArmed() const {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  return armed_;
}

bool SecurityCamera::isMotionDetected() const {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  return motionDetected_;
}

void SecurityCamera::simulateMotion(bool detected) {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  motionDetected_ = detected;
  if (detected && armed_) {
    time_t now = time(nullptr);
    lastAlert_ = "Motion detected at " + std::string(ctime(&now));
    // Remove trailing newline from ctime
    if (!lastAlert_.empty() && lastAlert_.back() == '\n') {
      lastAlert_.pop_back();
    }
  }
}

std::string SecurityCamera::getLastAlert() const {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  return lastAlert_;
}

std::string SecurityCamera::getDetailedStatus() const {
  std::lock_guard<std::mutex> lock(deviceMutex_);
  std::ostringstream oss;
  oss << "Device: " << name_ << " [" << id_ << "]\n"
      << "  Type: Security Camera\n"
      << "  Power: " << (powerOn_ ? "ON" : "OFF") << "\n"
      << "  Armed: " << (armed_ ? "YES" : "NO") << "\n"
      << "  Motion Detected: "
      << (motionDetected_ ? "YES" : "NO") << "\n"
      << "  Last Alert: " << lastAlert_ << "\n"
      << "  IP: " << ipAddress_ << "\n"
      << "  MAC: " << macAddress_ << "\n"
      << "  Subnet: " << subnet_;
  return oss.str();
}