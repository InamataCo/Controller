#pragma once

#include <Arduino.h>

#include <set>

#include "utils/color.h"

namespace bernd_box {
namespace peripheral {
namespace capabilities {

/**
 * A capability that allows a LED strip to be controlled
 */
class LedStrip {
 public:
  virtual ~LedStrip() = default;

  /**
   * Interface to turn the complete LED strip on to a specific color
   * 
   * \param color Color to turn the LED strip on to
   */
  virtual void turnOn(utils::Color color) = 0;

  /**
   * Interface to turn the LED strip off
   */
  virtual void turnOff() = 0;

  // Type checking
  static bool registerType(const String& type);
  static bool isSupported(const String& type);
  static const std::set<String>& getTypes();

 private:
  static std::set<String>& getSupportedTypes();
};

}  // namespace capabilities
}  // namespace peripheral
}  // namespace bernd_box
