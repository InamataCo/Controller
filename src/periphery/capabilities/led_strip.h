#pragma once

#include <Arduino.h>

#include <set>

#include "utils/color.h"

namespace bernd_box {
namespace periphery {
namespace capabilities {

class LedStrip {
 public:
  virtual ~LedStrip() = default;

  // Functions that have to be implemented to fulfill capability
  virtual void turnOn(utils::Color color) = 0;
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
