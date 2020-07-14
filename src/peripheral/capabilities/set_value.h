#pragma once

#include <Arduino.h>

#include <set>

namespace bernd_box {
namespace peripheral {
namespace capabilities {

/**
 * A capability that allows a unitless value to be set
 */
class SetValue {
 public:

  /**
   * Interface to set a unit-less value
   * 
   * \param value The value of the unitless value
   */
  virtual void setValue(float value) = 0;

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
