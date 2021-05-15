#pragma once

#include "managers/io.h"
#include "managers/mqtt.h"
#include "TaskSchedulerDeclarations.h"

namespace bernd_box {
namespace tasks {

/**
 * Reads, prints and then sends all light sensors
 */
class LightSensors : public Task {
 private:
  /// Name used in the MQTT topics (action and tele)
  const __FlashStringHelper* name_{F("light_sensors")};

  const std::chrono::seconds default_period_{1};
  static constexpr std::array<uint8_t, 2> bh1750_addresses_{{0x23, 0x5C}};
  static constexpr std::array<uint8_t, 2> max44009_addresses_{{0x4A, 0x4B}};

  struct SensorTypes {
    const __FlashStringHelper* bh1750 = {F("BH1750")};
    const __FlashStringHelper* max44009 = {F("MAX44009")};
  } sensor_types_;

 public:
  LightSensors(Scheduler* scheduler, Io& io, Mqtt& mqtt);
  virtual ~LightSensors();

 private:
  bool OnEnable() final;
  bool Callback() final;
  void OnDisable() final;
  void mqttCallback(char* topic, uint8_t* payload, unsigned int length);
  Result findSensors(const JsonObjectConst& doc);
  
  /**
   * {
   *   "name": "top_light_sensor",
   *   "i2c_interface": "main_interface",
   *   "address": 0xA8
   * }
   */
  Result addSensor(const JsonObjectConst& doc);

  Io& io_;
  Mqtt& mqtt_;
};

}  // namespace tasks
}  // namespace bernd_box
