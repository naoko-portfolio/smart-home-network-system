#ifndef SECURITY_CAMERA_H
#define SECURITY_CAMERA_H

#include "devices/Device.h"

class SecurityCamera : public Device {
 public:
  SecurityCamera(const std::string& id, const std::string& name,
                 const std::string& ipAddress,
                 const std::string& macAddress,
                 const std::string& subnet);
  ~SecurityCamera() override = default;

  // Security-specific controls
  void arm();
  void disarm();
  bool isArmed() const;
  bool isMotionDetected() const;
  void simulateMotion(bool detected);
  std::string getLastAlert() const;

  std::string getDetailedStatus() const override;

 private:
  bool armed_;
  bool motionDetected_;
  std::string lastAlert_;
};

#endif  // SECURITY_CAMERA_H