#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include "devices/Device.h"

class Thermostat : public Device {
 public:
  enum class Mode { OFF, HEAT, COOL, AUTO };

  Thermostat(const std::string& id, const std::string& name,
             const std::string& ipAddress,
             const std::string& macAddress,
             const std::string& subnet);
  ~Thermostat() override = default;

  // Thermostat-specific controls
  void setTargetTemperature(double temp);
  double getTargetTemperature() const;
  double getCurrentTemperature() const;
  void setMode(Mode mode);
  Mode getMode() const;
  std::string getModeString() const;

  std::string getDetailedStatus() const override;

 private:
  double targetTemperature_;   // in Celsius
  double currentTemperature_;  // simulated reading
  Mode mode_;
};

#endif  // THERMOSTAT_H