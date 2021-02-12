#include "calibrate.h"

namespace bernd_box {
namespace peripheral {
namespace capabilities {

bool Calibrate::registerType(const String& type) {
  return getSupportedTypes().insert(type).second;
}

bool Calibrate::isSupported(const String& type) {
  const std::set<String>& types = getSupportedTypes();
  return std::binary_search(types.begin(), types.end(), type);
}

const std::set<String>& Calibrate::getTypes() { return getSupportedTypes(); }

String Calibrate::invalidTypeError(const utils::UUID& uuid,
                                   std::shared_ptr<Peripheral> peripheral) {
  String error(F("Calibrate capability not supported: "));
  error += uuid.toString();
  error += F(" is a ");
  error += peripheral->getType();
  return error;
}

std::set<String>& Calibrate::getSupportedTypes() {
  static std::set<String> supported_types;
  return supported_types;
}

}  // namespace capabilities
}  // namespace peripheral
}  // namespace bernd_box
