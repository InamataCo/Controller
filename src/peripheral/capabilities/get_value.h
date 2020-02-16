#pragma once

#include "Arduino.h"

#include <set>

namespace bernd_box {
namespace peripheral {
namespace capabilities {

class GetValue {
 public:
  // Functions that have to be implemented to fulfill capability
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
