#include "value_unit.h"

namespace inamata {
namespace utils {

const String ValueUnit::sourceUnitError(const UUID& other_data_point_type) {
  return mismatchUnitError(other_data_point_type, data_point_type);
}

const String ValueUnit::targetUnitError(const UUID& other_data_point_type) {
  return mismatchUnitError(data_point_type, other_data_point_type);
}

const __FlashStringHelper* ValueUnit::value_key = FPSTR("value");
const __FlashStringHelper* ValueUnit::value_key_error =
    FPSTR("Missing property: value (String)");

const __FlashStringHelper* ValueUnit::data_point_type_key =
    FPSTR("data_point_type");
const __FlashStringHelper* ValueUnit::data_point_type_key_error =
    FPSTR("Missing property: data_point_type (String)");

const __FlashStringHelper* ValueUnit::data_points_key = FPSTR("data_points");

String ValueUnit::mismatchUnitError(const UUID& expected_data_point_type,
                                    const UUID& received_data_point_type) {
  String error(F("Mismatching unit. Got: "));
  error += received_data_point_type.toString();
  error += F(" instead of ");
  error += expected_data_point_type.toString();
  return error;
}

}  // namespace utils
}  // namespace inamata