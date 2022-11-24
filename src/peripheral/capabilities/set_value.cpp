#include "set_value.h"

namespace inamata {
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

String SetValue::invalidTypeError(const utils::UUID& uuid,
                                  std::shared_ptr<Peripheral> peripheral) {
  String error(F("SetValue capability not supported: "));
  error += uuid.toString();
  error += F(" is a ");
  error += peripheral->getType();
  return error;
}

std::set<String>& SetValue::getSupportedTypes() {
  static std::set<String> supported_types;
  return supported_types;
}

}  // namespace capabilities
}  // namespace peripheral
}  // namespace inamata