#pragma once

#include <Arduino.h>

#include <memory>
#include <set>

#include "types.h"

namespace bernd_box {
namespace peripheral {
namespace capabilities {

class GetValues {
 public:
  virtual std::vector<ValueUnit> getValues() = 0;

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