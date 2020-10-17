#include "types.h"

namespace bernd_box {
namespace peripheral {
namespace capabilities {

const String ValueUnit::sourceUnitError(const String& other_unit) {
  return mismatchUnitError(other_unit, unit);
}

const String ValueUnit::targetUnitError(const String& other_unit) {
  return mismatchUnitError(unit, other_unit);
}

const __FlashStringHelper* ValueUnit::value_key = F("value");
const __FlashStringHelper* ValueUnit::value_key_error =
    F("Missing property: value (String)");

const __FlashStringHelper* ValueUnit::unit_key = F("unit");
const __FlashStringHelper* ValueUnit::unit_key_error =
    F("Missing property: unit (String)");

String ValueUnit::mismatchUnitError(const String& expected_unit,
                                    const String& received_unit) {
  String error(F("Mismatching unit. Got: "));
  error += received_unit;
  error += F(" instead of ");
  error += expected_unit;
  return error;
}

}  // namespace capabilities
}  // namespace peripheral
}  // namespace bernd_box