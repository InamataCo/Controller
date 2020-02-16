#include "get_value.h"

namespace bernd_box {
namespace peripheral {
namespace capabilities {

bool GetValue::registerType(const String& type) {
  return getSupportedTypes().insert(type).second;
}

bool GetValue::isSupported(const String& type) {
  const std::set<String>& types = getSupportedTypes();
  return std::binary_search(types.begin(), types.end(), type);
}

const std::set<String>& GetValue::getTypes() { return getSupportedTypes(); }

std::set<String>& GetValue::getSupportedTypes() {
  static std::set<String> supported_types;
  return supported_types;
}

}  // namespace capabilities
}  // namespace peripheral
}  // namespace bernd_box
