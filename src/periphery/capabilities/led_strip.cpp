#include "led_strip.h"

namespace bernd_box {
namespace periphery {
namespace capabilities {

LedStrip::Color LedStrip::Color::fromRgbw(uint8_t r, uint8_t g, uint8_t b,
                                          uint8_t w) {
  return Color(r, g, b, w);
}

LedStrip::Color LedStrip::Color::fromBrightness(float percent) {
  uint8_t value = percent * 255;
  return Color(value, value, value, value);
}

LedStrip::Color::Color() : wrgb_(0) {}

LedStrip::Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
    : wrgb_((w << 24) | (r << 16) | (g << 8) | b) {}

uint32_t LedStrip::Color::getWrgbInt() { return wrgb_; }

bool LedStrip::registerType(const String& type) {
  return getSupportedTypes().insert(type).second;
}

bool LedStrip::isSupported(const String& type) {
  const std::set<String>& types = getSupportedTypes();
  return std::binary_search(types.begin(), types.end(), type);
}

const std::set<String>& LedStrip::getTypes() { return getSupportedTypes(); }

std::set<String>& LedStrip::getSupportedTypes() {
  static std::set<String> supported_types;
  return supported_types;
}

}  // namespace capabilities
}  // namespace periphery
}  // namespace bernd_box
