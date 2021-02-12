#include "start_measurement.h"

namespace bernd_box {
namespace peripheral {
namespace capabilities {

bool StartMeasurement::registerType(const String& type) {
  return getSupportedTypes().insert(type).second;
}

bool StartMeasurement::isSupported(const String& type) {
  const std::set<String>& types = getSupportedTypes();
  return std::binary_search(types.begin(), types.end(), type);
}

const std::set<String>& StartMeasurement::getTypes() {
  return getSupportedTypes();
}

String StartMeasurement::invalidTypeError(
    const utils::UUID& uuid, std::shared_ptr<Peripheral> peripheral) {
  String error(F("StartMeasurement capability not supported: "));
  error += uuid.toString();
  error += F(" is a ");
  error += peripheral->getType();
  return error;
}

std::set<String>& StartMeasurement::getSupportedTypes() {
  static std::set<String> supported_types;
  return supported_types;
}

}  // namespace capabilities
}  // namespace peripheral
}  // namespace bernd_box