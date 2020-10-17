#include "led_strip.h"

namespace bernd_box {
namespace peripheral {
namespace capabilities {

bool LedStrip::registerType(const String& type) {
  return getSupportedTypes().insert(type).second;
}

bool LedStrip::isSupported(const String& type) {
  const std::set<String>& types = getSupportedTypes();
  return std::binary_search(types.begin(), types.end(), type);
}

const std::set<String>& LedStrip::getTypes() { return getSupportedTypes(); }

String LedStrip::invalidTypeError(const UUID& uuid,
                                  std::shared_ptr<Peripheral> peripheral) {
  String error(F("LedStrip capability not supported: "));
  error += uuid.toString();
  error += String(F(" is a "));
  error += peripheral->getType();
  return error;
}

std::set<String>& LedStrip::getSupportedTypes() {
  static std::set<String> supported_types;
  return supported_types;
}

}  // namespace capabilities
}  // namespace peripheral
}  // namespace bernd_box
