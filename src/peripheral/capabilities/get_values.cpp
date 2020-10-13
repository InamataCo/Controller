#include "get_values.h"

namespace bernd_box {
namespace peripheral {
namespace capabilities {

bool GetValues::registerType(const String& type) {
  return getSupportedTypes().insert(type).second;
}

bool GetValues::isSupported(const String& type) {
  const std::set<String>& types = getSupportedTypes();
  return std::binary_search(types.begin(), types.end(), type);
}

const std::set<String>& GetValues::getTypes() { return getSupportedTypes(); }

std::set<String>& GetValues::getSupportedTypes() {
  static std::set<String> supported_types;
  return supported_types;
}

}  // namespace capabilities
}  // namespace peripheral
}  // namespace bernd_box