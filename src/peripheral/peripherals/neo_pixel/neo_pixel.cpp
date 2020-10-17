#include "neo_pixel.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace neo_pixel {

NeoPixel::NeoPixel(const JsonObjectConst& parameters) {
  JsonVariantConst color_encoding_str = parameters[color_encoding_key_];
  if (color_encoding_str.isNull() || !color_encoding_str.is<String>()) {
    setInvalid(color_encoding_key_error_);
    return;
  }

  JsonVariantConst led_pin = parameters[led_pin_key_];
  if (!led_pin.is<unsigned int>()) {
    setInvalid(led_pin_key_error_);
    return;
  }

  JsonVariantConst led_count = parameters[led_count_key_];
  if (!led_count.is<unsigned int>()) {
    setInvalid(led_count_key_error_);
    return;
  }

  uint8_t color_encoding_int = getColorEncoding(color_encoding_str.as<char*>());
  if (color_encoding_int == 0) {
    setInvalid(invalidColorEncodingError(color_encoding_str.as<char*>()));
    return;
  }

  uint8_t pixel_type = color_encoding_int + NEO_KHZ800;

  driver_.setPin(led_pin);
  driver_.updateType(pixel_type);
  driver_.updateLength(led_count);
}

const String& NeoPixel::getType() const { return type(); }

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

String NeoPixel::invalidColorEncodingError(const String& color_encoding) {
  String error(F("Invalid color encoding value: "));
  error += color_encoding;
  return error;
}

std::shared_ptr<Peripheral> NeoPixel::factory(
    const JsonObjectConst& parameter) {
  return std::make_shared<NeoPixel>(parameter);
}

bool NeoPixel::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

bool NeoPixel::capability_led_strip_ =
    capabilities::LedStrip::registerType(type());

const __FlashStringHelper* NeoPixel::color_encoding_key_ = F("color_encoding");
const __FlashStringHelper* NeoPixel::color_encoding_key_error_ F(
    "Missing property: color_encoding (string)");
const __FlashStringHelper* NeoPixel::led_pin_key_ = F("led_pin");
const __FlashStringHelper* NeoPixel::led_pin_key_error_ =
    F("Missing property: led_pin (unsigned int)");
const __FlashStringHelper* NeoPixel::led_count_key_ = F("led_count");
const __FlashStringHelper* NeoPixel::led_count_key_error_ F(
    "Missing property: led_count (unsigned int)");

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
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
