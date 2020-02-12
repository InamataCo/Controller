#pragma once

#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

#include <memory>

#include "managers/services.h"
#include "periphery/abstract_periphery.h"
#include "periphery/capabilities/led_strip.h"

namespace bernd_box {
namespace periphery {
namespace peripheries {
namespace neo_pixel {

class NeoPixel : public AbstractPeriphery, public capabilities::LedStrip {
 public:
  NeoPixel(const JsonObjectConst& parameters);
  virtual ~NeoPixel() = default;

  // Type registration in the periphery factory / library
  const String& getType() final;
  static const String& type();

  void turnOn(Color color) final;
  void turnOff() final;

 private:
  static std::shared_ptr<Periphery> factory(const JsonObjectConst& parameter);
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
};

}  // namespace neo_pixel
}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box
