#ifndef LIGHT_H
#define LIGHT_H

#include "devices/Device.h"

class Light : public Device {
 public:
  Light(const std::string& id, const std::string& name,
        const std::string& ipAddress, const std::string& macAddress,
        const std::string& subnet);
  ~Light() override = default;

  // Light-specific controls
  void setBrightness(int level);  // 0-100
  int getBrightness() const;
  void setColor(const std::string& color);
  std::string getColor() const;

  std::string getDetailedStatus() const override;

 private:
  int brightness_;           // 0-100
  std::string color_;        // e.g., "white", "warm", "cool"
};

#endif  // LIGHT_H