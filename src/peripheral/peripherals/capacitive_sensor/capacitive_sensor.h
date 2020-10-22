#pragma once

#include <ArduinoJson.h>

#include <memory>

#include "peripheral/capabilities/get_values.h"
#include "peripheral/peripheral.h"
#include "peripheral/peripheral_factory.h"
#include "utils/value_unit.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace capacative_sensor {

/**
 * Peripheral to read capacitive sensors
 */
class CapacitiveSensor : public Peripheral, public capabilities::GetValues {
 public:
  CapacitiveSensor(const JsonObjectConst& parameters);
  virtual ~CapacitiveSensor() = default;

  // Type registration in the peripheral factory
  const String& getType() const final;
  static const String& type();

  /**
   * Read touch pad (values close to 0 mean touch detected)
   *
   * \return Value of the touch pad sensor
   */
  std::vector<utils::ValueUnit> getValues() final;

 private:
  static std::shared_ptr<Peripheral> factory(const JsonObjectConst& parameters);
  static bool registered_;
  static bool capability_get_value_;

  unsigned int sense_pin_;
  utils::UUID data_point_type_;

  /// Name of parameter for the pin # to measure capacitance
  static const __FlashStringHelper* sense_pin_key_;
  static const __FlashStringHelper* sense_pin_key_error_;
};

}  // namespace capacative_sensor
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
