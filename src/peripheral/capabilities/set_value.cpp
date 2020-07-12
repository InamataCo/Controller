#include "set_value.h"

namespace bernd_box {
namespace peripheral {
namespace capabilities {

bool SetValue::registerType(const String& type) {
  return getSupportedTypes().insert(type).second;
}

bool SetValue::isSupported(const String& type) {
  const std::set<String>& types = getSupportedTypes();
  return std::binary_search(types.begin(), types.end(), type);
}

const std::set<String>& SetValue::getTypes() { return getSupportedTypes(); }

std::set<String>& SetValue::getSupportedTypes() {
  static std::set<String> supported_types;
  return supported_types;
}

}  // namespace capabilities
}  // namespace peripheral
}  // namespace bernd_box