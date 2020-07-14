#pragma once

#include <Arduino.h>

#include <set>

namespace bernd_box {
namespace peripheral {
namespace capabilities {

/**
 * A capability that allows a value to be returned
 */
class GetValue {
 public:
  /**
   * Returns a unitless float value
   * 
   * \return The unitless value
   */
  virtual float getValue() = 0;

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
