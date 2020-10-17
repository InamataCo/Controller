#pragma once

#include <ArduinoJson.h>

#include "managers/services.h"
#include "peripheral/capabilities/set_value.h"
#include "peripheral/peripheral.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace pump {

/**
 * Peripheral to control a pump
 */
class PumpActuator : public Peripheral, public capabilities::SetValue {
 public:
  PumpActuator(const JsonObjectConst& parameters);
  virtual ~PumpActuator() = default;

  // Type registration in the peripheral factory
  const String& getType() const final;
  static const String& type();

  /**
   * Turns a pump on or off
   * 
   * \param value 1 turns the pump on, 0 turns it off
   */
  void setValue(capabilities::ValueUnit value_unit) final;

 private:
  static std::shared_ptr<Peripheral> factory(const JsonObjectConst& parameter);
  static bool registered_;
  static bool capability_set_value_;
  static const __FlashStringHelper* set_value_unit_;

  /// Name of the parameter for the pump to control a pump
  static const __FlashStringHelper* pump_pin_key_;
  static const __FlashStringHelper* pump_pin_key_error_;

  unsigned int pump_pin_;
};

}  // namespace pump
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
