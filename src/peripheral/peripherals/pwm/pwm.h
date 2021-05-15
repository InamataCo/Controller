#pragma once

#include <ArduinoJson.h>

#include <bitset>
#include <memory>

#include "managers/service_getters.h"
#include "peripheral/capabilities/set_value.h"
#include "peripheral/peripheral.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace pwm {

class Pwm : public Peripheral, public capabilities::SetValue {
 public:
  Pwm(const ServiceGetters& services, const JsonObjectConst& parameters);
  virtual ~Pwm();

  // Type registration in the peripheral factory
  const String& getType() const final;
  static const String& type();

  /**
   * Turn on the connected PWM signal to the specified value
   *
   * \param value A value between 0 and 1 sets the percentage brightness
   */
  void setValue(utils::ValueUnit value_unit);

 private:
  /**
   * Reserves and sets up a free PWM channel
   *
   * \param pin Pin number with which the PWM signal is connected
   * \param frequency Frequency of the PWM signal
   * \param resolution Resolution of the duty cycle
   * \return True if an error occured while setting up the channel
   */
  bool setup(uint pin, uint frequency = 5000, uint resolution = 8);

  /**
   * Frees the configured PWM channel and pin
   */
  void freeResources();

  static std::shared_ptr<Peripheral> factory(const ServiceGetters& services,
                                             const JsonObjectConst& parameters);
  static bool registered_;
  static bool capability_set_value_;

  /// Interface to send data to the server
  std::shared_ptr<Server> server_;

  /// Marks which PWM channels are currently in use
  static std::bitset<16> busy_channels_;

  utils::UUID data_point_type_{nullptr};

  int pin_ = -1;
  int channel_ = -1;
  int resolution_ = -1;

  /// Name of the parameter to which the pin the PWM signal is connected
  static const __FlashStringHelper* pin_key_;
  static const __FlashStringHelper* pin_key_error_;
  static const __FlashStringHelper* no_channels_available_error_;
};

}  // namespace pwm
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box