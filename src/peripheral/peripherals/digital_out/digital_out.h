#pragma once

#include <ArduinoJson.h>

#include "managers/service_getters.h"
#include "peripheral/capabilities/set_value.h"
#include "peripheral/peripheral.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace digital_out {

/**
 * Peripheral to control a GPIO output
 */
class DigitalOut : public Peripheral, public capabilities::SetValue {
 public:
  DigitalOut(const ServiceGetters& services, const JsonObjectConst& parameters);
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
  static std::shared_ptr<Peripheral> factory(const ServiceGetters& services,
                                             const JsonObjectConst& parameter);
  static bool registered_;
  static bool capability_set_value_;

  /// Interface to send data to the server
  std::shared_ptr<Server> server_;

  /// The pin to be used as a GPIO output
  unsigned int pin_;
  static const __FlashStringHelper* pin_key_;
  static const __FlashStringHelper* pin_key_error_;

  // Check if an initial state should be set
  static const __FlashStringHelper* initial_state_key_;
  static const __FlashStringHelper* initial_state_key_error_;

  /// If active low logic is used. Inverts set value. False by default
  bool active_low_ = false;
  static const __FlashStringHelper* active_low_key_;
  static const __FlashStringHelper* active_low_key_error_;

  /// Data point type for the GPIO output pin state
  utils::UUID data_point_type_{nullptr};
};

}  // namespace digital_out
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata
