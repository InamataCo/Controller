#pragma once

#include <ArduinoJson.h>

#include <memory>

#include "managers/services.h"
#include "periphery/abstract_periphery.h"
#include "periphery/capabilities/get_value.h"

namespace bernd_box {
namespace periphery {
namespace peripheries {
namespace capacative_sensor {

class CapacitiveSensor : public AbstractPeriphery,
                         public capabilities::GetValue {
 public:
  CapacitiveSensor(const JsonObjectConst& parameter);
  virtual ~CapacitiveSensor() = default;

  // Type registration in the periphery factory / library
  const String& getType() final;
  static const String& type();

  // Capability: GetValue
  float getValue() final;

  /// Name of parameter for the pin # to measure capacitance
  static const __FlashStringHelper* sense_pin_name_;

 private:
  static std::shared_ptr<Periphery> factory(const JsonObjectConst& parameter);
  static bool registered_;
  static bool capability_get_value_;

  unsigned int sense_pin_;
};

}  // namespace capacative_sensor
}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box
