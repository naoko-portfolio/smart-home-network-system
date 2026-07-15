#ifndef DEVICE_H
#define DEVICE_H

#include <mutex>
#include <string>

// Base class for all smart home devices
class Device {
 public:
  enum class DeviceType { LIGHT, THERMOSTAT, SECURITY_CAMERA };

  Device(const std::string& id, const std::string& name,
         DeviceType type, const std::string& ipAddress,
         const std::string& macAddress, const std::string& subnet);
  virtual ~Device() = default;

  // Getters
  std::string getId() const;
  std::string getName() const;
  DeviceType getType() const;
  std::string getTypeString() const;
  bool isOn() const;
  std::string getIpAddress() const;
  std::string getMacAddress() const;
  std::string getSubnet() const;

  // Controls
  virtual void turnOn();
  virtual void turnOff();
  virtual std::string getStatus() const;

  // Thread-safe status report
  virtual std::string getDetailedStatus() const = 0;

 protected:
  mutable std::mutex deviceMutex_;
  std::string id_;
  std::string name_;
  DeviceType type_;
  bool powerOn_;
  std::string ipAddress_;
  std::string macAddress_;
  std::string subnet_;
};

#endif  // DEVICE_H