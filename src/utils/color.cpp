#include "color.h"

namespace inamata {
namespace utils {

size_t Color::printTo(Print& p) const {
  return p.printf("r: %u, g: %u, b: %u, w: %u", getRed(), getGreen(), getBlue(),
                  getWhite());
}

Color Color::fromRgbw(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  return Color(r, g, b, w);
}

Color Color::fromBrightness(float percent) {
  uint8_t value = percent * 255;
  return Color(value, value, value, value);
}

Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
    : wrgb_((w << 24) | (r << 16) | (g << 8) | b) {}

uint32_t Color::getWrgbInt() const { return wrgb_; }

uint8_t Color::getRed() const { return (wrgb_ & 0x00FF0000) >> 16; }

uint8_t Color::getGreen() const { return (wrgb_ & 0x0000FF00) >> 8; }

uint8_t Color::getBlue() const { return wrgb_ & 0x000000FF; }

uint8_t Color::getWhite() const { return (wrgb_ & 0xFF000000) >> 24; }

}  // namespace utils
}  // namespace inamata
