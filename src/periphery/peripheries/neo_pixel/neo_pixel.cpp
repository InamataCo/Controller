#include "neo_pixel.h"

namespace bernd_box {
namespace periphery {
namespace peripheries {
namespace neo_pixel {

NeoPixel::NeoPixel(const JsonObjectConst& parameters) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  JsonVariantConst color_encoding_str = parameters[keys_.color_encoding];
  if (color_encoding_str.isNull() || !color_encoding_str.is<String>()) {
    Services::getMqtt().sendError(who, String(F("Missing property: ")) +
                                           keys_.color_encoding +
                                           F(" (string)"));
    setInvalid();
    return;
  }

  JsonVariantConst led_pin = parameters[keys_.led_pin];
  if (!led_pin.is<unsigned int>()) {
    Services::getMqtt().sendError(
        who,
        String(F("Missing property: ")) + keys_.led_pin + F(" (unsigned int)"));
    setInvalid();
    return;
  }

  JsonVariantConst led_count = parameters[keys_.led_count];
  if (!led_count.is<unsigned int>()) {
    Services::getMqtt().sendError(who, String(F("Missing property: ")) +
                                           keys_.led_count +
                                           F(" (unsigned int)"));
    setInvalid();
    return;
  }

  uint8_t color_encoding_int = getColorEncoding(color_encoding_str.as<char*>());
  if (color_encoding_int == 0) {
    Services::getMqtt().sendError(
        who, String(F("Invalid color_encoding value: ")) + color_encoding_int);
    setInvalid();
    return;
  }

  uint8_t pixel_type = color_encoding_int + NEO_KHZ800;

  driver_.setPin(led_pin);
  driver_.updateType(pixel_type);
  driver_.updateLength(led_count);
}

const String& NeoPixel::getType() { return type(); }

const String& NeoPixel::type() {
  static const String name{"NeoPixel"};
  return name;
}

void NeoPixel::turnOn(utils::Color color) {
  if (!is_driver_started_) {
    driver_.begin();
    is_driver_started_ = true;
  }
  driver_.fill(color.getWrgbInt());
  driver_.show();
}

void NeoPixel::turnOff() {
  driver_.setBrightness(0);
  driver_.show();
}

std::shared_ptr<Periphery> NeoPixel::factory(const JsonObjectConst& parameter) {
  return std::make_shared<NeoPixel>(parameter);
}

bool NeoPixel::registered_ = PeripheryFactory::registerFactory(type(), factory);

bool NeoPixel::capability_led_strip_ =
    capabilities::LedStrip::registerType(type());

NeoPixel::Keys NeoPixel::keys_;

uint8_t NeoPixel::getColorEncoding(String encoding_str) {
  // Check that it is a valid rgb_encoding
  bool isValid = cleanColorEncoding(encoding_str);
  if (!isValid) {
    return 0;
  }

  uint8_t neo_encoding = 0;

  // Transform to rgb/rgbw permutations in Adafruit_NeoPixel.h
  // Skip first color, as 0 shifted is always 0
  for (int i = 1; i < encoding_str.length(); i++) {
    if (encoding_str[i] == 'b') {
      neo_encoding |= i << blue_offset_;
      continue;
    }
    if (encoding_str[i] == 'g') {
      neo_encoding |= i << green_offset_;
      continue;
    }
    if (encoding_str[i] == 'r') {
      neo_encoding |= i << red_offset_;

      // For 3 color encodings, use the red value for white
      if (encoding_str.length() == 3) {
        neo_encoding |= i << white_offset_;
      }
      continue;
    }
    if (encoding_str[i] == 'w') {
      neo_encoding |= i << white_offset_;
      continue;
    }
  }

  return neo_encoding;
}

bool NeoPixel::cleanColorEncoding(String& color_encoding) {
  std::transform(color_encoding.begin(), color_encoding.end(),
                 color_encoding.begin(), ::tolower);

  String sorted_encoding = color_encoding;
  std::sort(sorted_encoding.begin(), sorted_encoding.end());

  if (sorted_encoding != "bgr" && sorted_encoding != "bgrw") {
    return false;
  } else {
    return true;
  }
}

}  // namespace neo_pixel
}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box
