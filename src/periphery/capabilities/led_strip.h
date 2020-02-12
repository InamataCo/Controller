#pragma once

#include <set>

#include "Arduino.h"

namespace bernd_box {
namespace periphery {
namespace capabilities {

class LedStrip {
 public:
  class Color {
   public:
    static Color fromRgbw(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0);
    static Color fromBrightness(float percent);

    uint32_t getWrgbInt();

   private:
    Color();
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w);

    uint32_t wrgb_;
  };

  // Functions that have to be implemented to fulfill capability
  virtual void turnOn(Color color) = 0;
  virtual void turnOff() = 0;

  // Type checking
  static bool registerType(const String& type);
  static bool isSupported(const String& type);
  static const std::set<String>& getTypes();

 private:
  static std::set<String>& getSupportedTypes();
};

}  // namespace capabilities
}  // namespace periphery
}  // namespace bernd_box
