#pragma once

#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

#include <memory>

#include "managers/service_getters.h"
#include "peripheral/capabilities/led_strip.h"
#include "peripheral/peripheral.h"
#include "utils/color.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace neo_pixel {

/**
 * A peripheral to control NeoPixels
 */
class NeoPixel : public Peripheral, public capabilities::LedStrip {
 public:
  NeoPixel(const JsonObjectConst& parameters);
  virtual ~NeoPixel() = default;

  // Type registration in the peripheral factory
  const String& getType() const final;
  static const String& type();

  /**
   * Turns on all LEDs in a strip to a specific color
   *
   * \param color Color of the LEDs
   */
  void turnOn(utils::Color color) final;

  /**
   * Turn off all LEDs
   */
  void turnOff() final;

 private:
  static String invalidColorEncodingError(const String& color_encoding);

  static std::shared_ptr<Peripheral> factory(const ServiceGetters& services,
                                             const JsonObjectConst& parameters);
  static bool registered_;
  static bool capability_led_strip_;

  static const uint8_t blue_offset_{0};
  static const uint8_t green_offset_{2};
  static const uint8_t red_offset_{4};
  static const uint8_t white_offset_{6};

  static const __FlashStringHelper* color_encoding_key_;
  static const __FlashStringHelper* color_encoding_key_error_;
  static const __FlashStringHelper* led_pin_key_;
  static const __FlashStringHelper* led_pin_key_error_;
  static const __FlashStringHelper* led_count_key_;
  static const __FlashStringHelper* led_count_key_error_;

  uint8_t getColorEncoding(String color_encoding);
  bool cleanColorEncoding(String& color_encoding);

  Adafruit_NeoPixel driver_;
  bool is_driver_started_ = false;
};

}  // namespace neo_pixel
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
