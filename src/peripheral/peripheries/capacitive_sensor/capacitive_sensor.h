#pragma once

#include <ArduinoJson.h>

#include <memory>

#include "managers/services.h"
#include "peripheral/peripheral.h"
#include "peripheral/capabilities/get_value.h"

namespace bernd_box {
namespace peripheral {
namespace peripheries {
namespace capacative_sensor {

class CapacitiveSensor : public Peripheral,
                         public capabilities::GetValue {
 public:
  CapacitiveSensor(const JsonObjectConst& parameters);
  virtual ~CapacitiveSensor() = default;

  // Type registration in the periphery factory / library
  const String& getType() final;
  static const String& type();

  // Capability: GetValue
  float getValue() final;

  /// Name of parameter for the pin # to measure capacitance
  static const __FlashStringHelper* sense_pin_name_;

 private:
  static std::shared_ptr<Peripheral> factory(const JsonObjectConst& parameter);
  static bool registered_;
  static bool capability_get_value_;

  unsigned int sense_pin_;
};

}  // namespace capacative_sensor
}  // namespace peripheries
}  // namespace peripheral
}  // namespace bernd_box
