#include "led_strip.h"

namespace bernd_box {
namespace periphery {
namespace capabilities {

bool LedStrip::registerType(const String& type) {
  return getSupportedTypes().insert(type).second;
}

bool LedStrip::isSupported(const String& type) {
  const std::set<String>& types = getSupportedTypes();
  return std::binary_search(types.begin(), types.end(), type);
}

const std::set<String>& LedStrip::getTypes() { return getSupportedTypes(); }

std::set<String>& LedStrip::getSupportedTypes() {
  static std::set<String> supported_types;
  return supported_types;
}

}  // namespace capabilities
}  // namespace periphery
}  // namespace bernd_box
