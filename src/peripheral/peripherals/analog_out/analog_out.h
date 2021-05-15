#pragma once

#include <ArduinoJson.h>

#include "managers/service_getters.h"
#include "peripheral/capabilities/set_value.h"
#include "peripheral/peripheral.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace analog_out {

/**
 * Peripheral to control a GPIO output
 */
class AnalogOut : public Peripheral, public capabilities::SetValue {
 public:
  AnalogOut(const ServiceGetters& services, const JsonObjectConst& parameters);
  virtual ~AnalogOut() = default;

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
  static const __FlashStringHelper* invalid_pin_error_;

  /// Data point type for the writing as voltage
  utils::UUID voltage_data_point_type_{nullptr};
  static const __FlashStringHelper* voltage_data_point_type_key_;
  /// Data point type for the writing as percent
  utils::UUID percent_data_point_type_{nullptr};
  static const __FlashStringHelper* percent_data_point_type_key_;
  /// Error if neither percent nor voltage data point types are set
  static const __FlashStringHelper* data_point_type_key_error_;
};

}  // namespace analog_out
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
