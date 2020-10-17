#pragma once

#include <ArduinoJson.h>

#include <memory>
#include <bitset>

#include "managers/services.h"
#include "peripheral/capabilities/set_value.h"
#include "peripheral/peripheral.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace led {

class Led : public Peripheral, public capabilities::SetValue {
 public:
  Led(const JsonObjectConst& parameters);
  virtual ~Led();

  // Type registration in the peripheral factory
  const String& getType() const final;
  static const String& type();

  /**
   * Turn on the connected LED to the specified value
   * 
   * \param value A value between 0 and 1 sets the percentage brightness
   */
  void setValue(capabilities::ValueUnit value_unit);

 private:
  /**
   * Reserves and sets up a free LED channel
   * 
   * \param pin Pin number with which the LED is connected
   * \param freq Frequency of the PWM signal
   * \param resolution Resolution of the duty cycle
   * \return True if an error occured while setting up the channel
   */
  bool setup(uint pin, uint freq = 5000, uint resolution = 8);

  /**
   * Frees the configured LED channel and pin
   */
  void freeResources();

  static std::shared_ptr<Peripheral> factory(const JsonObjectConst& parameters);
  static bool registered_;
  static bool capability_set_value_;
  static const __FlashStringHelper* set_value_unit_;

  /// Name of the parameter to which the pin the LED is connected
  static const __FlashStringHelper* led_pin_key_;
  static const __FlashStringHelper* led_pin_key_error_;
  static const __FlashStringHelper* no_channels_available_error_;

  /// Marks which LED channels are currently in use
  static std::bitset<16> busy_led_channels_;

  int led_pin_ = -1;
  int led_channel_ = -1;
  int resolution_ = -1;
};

}  // namespace led
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box