#include <cassert>
#include <iostream>
#include <string>

#include "devices/Device.h"
#include "devices/Light.h"
#include "devices/SecurityCamera.h"
#include "devices/Thermostat.h"

static int passed = 0;
static int failed = 0;

#define TEST(name, expr)                                       \
  do {                                                         \
    if (expr) {                                                \
      std::cout << "  PASS: " << name << std::endl;            \
      passed++;                                                \
    } else {                                                   \
      std::cout << "  FAIL: " << name << std::endl;            \
      failed++;                                                \
    }                                                          \
  } while (0)

void testLight() {
  std::cout << "\n=== Light Tests ===" << std::endl;
  Light light("light-1", "Test Light", "192.168.1.2",
              "AA:BB:CC:DD:EE:01", "Lighting");

  TEST("Light initial power is OFF", light.isOn() == false);
  TEST("Light type is LIGHT",
       light.getType() == Device::DeviceType::LIGHT);
  TEST("Light type string", light.getTypeString() == "Light");
  TEST("Light initial brightness is 100",
       light.getBrightness() == 100);
  TEST("Light initial color is white",
       light.getColor() == "white");
  TEST("Light IP address", light.getIpAddress() == "192.168.1.2");
  TEST("Light MAC address",
       light.getMacAddress() == "AA:BB:CC:DD:EE:01");
  TEST("Light subnet", light.getSubnet() == "Lighting");

  // Turn on
  light.turnOn();
  TEST("Light turn on", light.isOn() == true);
  TEST("Light status after on", light.getStatus() == "ON");

  // Turn off
  light.turnOff();
  TEST("Light turn off", light.isOn() == false);
  TEST("Light status after off", light.getStatus() == "OFF");

  // Brightness
  light.setBrightness(50);
  TEST("Light set brightness to 50", light.getBrightness() == 50);

  light.setBrightness(0);
  TEST("Light set brightness to 0", light.getBrightness() == 0);

  light.setBrightness(100);
  TEST("Light set brightness to 100",
       light.getBrightness() == 100);

  // Boundary: clamp below 0
  light.setBrightness(-10);
  TEST("Light brightness clamped at 0",
       light.getBrightness() == 0);

  // Boundary: clamp above 100
  light.setBrightness(200);
  TEST("Light brightness clamped at 100",
       light.getBrightness() == 100);

  // Color
  light.setColor("warm");
  TEST("Light set color to warm", light.getColor() == "warm");

  // Detailed status contains key info
  light.turnOn();
  std::string status = light.getDetailedStatus();
  TEST("Detailed status contains name",
       status.find("Test Light") != std::string::npos);
  TEST("Detailed status contains ON",
       status.find("ON") != std::string::npos);
  TEST("Detailed status contains IP",
       status.find("192.168.1.2") != std::string::npos);
}

void testThermostat() {
  std::cout << "\n=== Thermostat Tests ===" << std::endl;
  Thermostat thermo("thermo-1", "Test Thermostat", "192.168.1.66",
                    "AA:BB:CC:DD:EE:02", "Thermostat");

  TEST("Thermostat initial power is OFF", thermo.isOn() == false);
  TEST("Thermostat type is THERMOSTAT",
       thermo.getType() == Device::DeviceType::THERMOSTAT);
  TEST("Thermostat type string",
       thermo.getTypeString() == "Thermostat");
  TEST("Thermostat initial target temp is 22.0",
       thermo.getTargetTemperature() == 22.0);
  TEST("Thermostat initial current temp is 20.0",
       thermo.getCurrentTemperature() == 20.0);
  TEST("Thermostat initial mode is AUTO",
       thermo.getMode() == Thermostat::Mode::AUTO);
  TEST("Thermostat mode string", thermo.getModeString() == "AUTO");

  // Set temperature
  thermo.setTargetTemperature(25.0);
  TEST("Set target temp to 25", thermo.getTargetTemperature() == 25.0);

  // Boundary: clamp below 10
  thermo.setTargetTemperature(5.0);
  TEST("Temp clamped at 10", thermo.getTargetTemperature() == 10.0);

  // Boundary: clamp above 35
  thermo.setTargetTemperature(40.0);
  TEST("Temp clamped at 35", thermo.getTargetTemperature() == 35.0);

  // Set mode
  thermo.setMode(Thermostat::Mode::HEAT);
  TEST("Set mode to HEAT",
       thermo.getMode() == Thermostat::Mode::HEAT);
  TEST("Mode string HEAT", thermo.getModeString() == "HEAT");

  thermo.setMode(Thermostat::Mode::COOL);
  TEST("Set mode to COOL",
       thermo.getMode() == Thermostat::Mode::COOL);

  // Detailed status
  thermo.turnOn();
  std::string status = thermo.getDetailedStatus();
  TEST("Detailed status contains name",
       status.find("Test Thermostat") != std::string::npos);
  TEST("Detailed status contains Thermostat",
       status.find("Thermostat") != std::string::npos);
}

void testSecurityCamera() {
  std::cout << "\n=== Security Camera Tests ===" << std::endl;
  SecurityCamera cam("cam-1", "Test Camera", "192.168.1.98",
                     "AA:BB:CC:DD:EE:03", "Security");

  TEST("Camera initial power is OFF", cam.isOn() == false);
  TEST("Camera type is SECURITY_CAMERA",
       cam.getType() == Device::DeviceType::SECURITY_CAMERA);
  TEST("Camera type string",
       cam.getTypeString() == "SecurityCamera");
  TEST("Camera initial armed is false", cam.isArmed() == false);
  TEST("Camera initial motion is false",
       cam.isMotionDetected() == false);
  TEST("Camera initial alert is None",
       cam.getLastAlert() == "None");

  // Arm
  cam.arm();
  TEST("Camera arm sets armed", cam.isArmed() == true);
  TEST("Camera arm turns on power", cam.isOn() == true);

  // Simulate motion while armed
  cam.simulateMotion(true);
  TEST("Motion detected when armed",
       cam.isMotionDetected() == true);
  TEST("Alert generated on motion",
       cam.getLastAlert() != "None");
  TEST("Alert contains 'Motion detected'",
       cam.getLastAlert().find("Motion detected") !=
           std::string::npos);

  // Disarm
  cam.disarm();
  TEST("Camera disarm sets unarmed", cam.isArmed() == false);
  TEST("Camera disarm clears motion",
       cam.isMotionDetected() == false);

  // Motion while disarmed should not trigger alert
  std::string alertBefore = cam.getLastAlert();
  cam.simulateMotion(true);
  TEST("No new alert when disarmed",
       cam.getLastAlert() == alertBefore);

  // Detailed status
  std::string status = cam.getDetailedStatus();
  TEST("Detailed status contains name",
       status.find("Test Camera") != std::string::npos);
  TEST("Detailed status contains Security Camera",
       status.find("Security Camera") != std::string::npos);
}

int main() {
  std::cout << "========== Device Tests ==========" << std::endl;

  testLight();
  testThermostat();
  testSecurityCamera();

  std::cout << "\n========== Results ==========" << std::endl;
  std::cout << "Passed: " << passed << std::endl;
  std::cout << "Failed: " << failed << std::endl;

  return (failed == 0) ? 0 : 1;
}
