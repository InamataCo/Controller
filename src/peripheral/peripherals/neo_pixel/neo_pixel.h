#pragma once

#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

#include <memory>

#include "managers/services.h"
#include "peripheral/peripheral.h"
#include "peripheral/capabilities/led_strip.h"
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
  const String& getType() final;
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
  static std::shared_ptr<Peripheral> factory(const JsonObjectConst& parameters);
  static bool registered_;
  static bool capability_led_strip_;

  static const uint8_t blue_offset_{0};
  static const uint8_t green_offset_{2};
  static const uint8_t red_offset_{4};
  static const uint8_t white_offset_{6};

  static struct Keys {
    const __FlashStringHelper* color_encoding = F("color_encoding");
    const __FlashStringHelper* led_pin = F("led_pin");
    const __FlashStringHelper* led_count = F("led_count");
  } keys_;

  uint8_t getColorEncoding(String color_encoding);
  bool cleanColorEncoding(String& color_encoding);

  Adafruit_NeoPixel driver_;
  bool is_driver_started_ = false;
};

}  // namespace neo_pixel
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
