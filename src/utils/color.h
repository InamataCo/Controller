#pragma once

#include "Arduino.h"

namespace bernd_box {
namespace utils {

class Color : public Printable {
 public:
  Color() = default;
  virtual ~Color() = default;

  size_t printTo(Print& p) const;  

  static Color fromRgbw(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0);
  static Color fromBrightness(float percent);

  uint32_t getWrgbInt() const;
  
  uint8_t getRed() const;
  uint8_t getGreen() const;
  uint8_t getBlue() const;
  uint8_t getWhite() const;

 private:
  Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w);

  uint32_t wrgb_;
};

}  // namespace utils

}  // namespace bernd_box
