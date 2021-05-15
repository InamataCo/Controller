#pragma once

#include <ArduinoJson.h>

#include <memory>

#include "managers/service_getters.h"
#include "peripheral/capabilities/get_values.h"
#include "peripheral/peripheral.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace analog_in {

/**
 * Peripheral to control a GPIO output
 */
class AnalogIn : public Peripheral, public capabilities::GetValues {
 public:
  AnalogIn(const JsonObjectConst& parameters);
  virtual ~AnalogIn() = default;

  // Type registration in the peripheral factory
  const String& getType() const final;
  static const String& type();

  /**
   * Get the GPIO state
   *
   * \return The value 1 represents the high state, 0 its low state
   */
  capabilities::GetValues::Result getValues() final;

 private:
  static std::shared_ptr<Peripheral> factory(const ServiceGetters& services,
                                             const JsonObjectConst& parameter);
  static bool registered_;
  static bool capability_get_values_;

  /// The pin to be used as a GPIO output
  unsigned int pin_;
  static const std::array<uint8_t, 8> valid_pins_;
  static const __FlashStringHelper* pin_key_;
  static const __FlashStringHelper* pin_key_error_;
  static const __FlashStringHelper* invalid_pin_error_;

  /// Data point type for the reading as voltage
  utils::UUID voltage_data_point_type_{nullptr};
  static const __FlashStringHelper* voltage_data_point_type_key_;
  /// Data point type for the reading as percent
  utils::UUID percent_data_point_type_{nullptr};
  static const __FlashStringHelper* percent_data_point_type_key_;
  /// Error if neither percent nor voltage data point types are set
  static const __FlashStringHelper* data_point_type_key_error_;
};

}  // namespace analog_in
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
