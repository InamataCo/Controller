#pragma once

#include <ArduinoJson.h>

#include "managers/services.h"
#include "peripheral/capabilities/set_value.h"
#include "peripheral/peripheral.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace digital_out {

/**
 * Peripheral to control a GPIO output
 */
class DigitalOut : public Peripheral, public capabilities::SetValue {
 public:
  DigitalOut(const JsonObjectConst& parameters);
  virtual ~DigitalOut() = default;

  // Type registration in the peripheral factory
  const String& getType() const final;
  static const String& type();

  /**
   * Turns the GPIO on or off
   *
   * \param value 1 sets the pin to its high state, 0 to its low state
   */
  void setValue(utils::ValueUnit value_unit) final;

 private:
  static std::shared_ptr<Peripheral> factory(const JsonObjectConst& parameter);
  static bool registered_;
  static bool capability_set_value_;

  /// The pin to be used as a GPIO output
  unsigned int pin_;
  static const __FlashStringHelper* pin_key_;
  static const __FlashStringHelper* pin_key_error_;

  /// Data point type for the GPIO output pin state
  utils::UUID data_point_type_{nullptr};
};

}  // namespace digital_out
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
