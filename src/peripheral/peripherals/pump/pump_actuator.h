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
  void setValue(utils::ValueUnit value_unit) final;

 private:
  static std::shared_ptr<Peripheral> factory(const JsonObjectConst& parameter);
  static bool registered_;
  static bool capability_set_value_;

  /// Name of the parameter for the pump to control a pump
  unsigned int pump_pin_;
  static const __FlashStringHelper* pump_pin_key_;
  static const __FlashStringHelper* pump_pin_key_error_;

  /// Data point type for the pump states
  utils::UUID pump_state_data_point_type_{nullptr};
  static const __FlashStringHelper* pump_state_data_point_type_key_;
  static const __FlashStringHelper* pump_state_data_point_type_key_error_;
};

}  // namespace pump
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
