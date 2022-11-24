#pragma once

#include <Arduino.h>

#include "utils/uuid.h"

namespace inamata {
namespace utils {

struct ValueUnit {
  float value;
  UUID data_point_type;

  /**
   * Error for mismatching units when ValueUnit has the wrong unit
   *
   * \param other_data_point_type The correct unit
   * \return Error string for mismatching types
   */
  const String sourceUnitError(const UUID& other_data_point_type);

  /**
   * Error for mismatching units when ValueUnit has the correct unit
   *
   * \param other_data_point_type The wrong unit
   * \return Error string for mismatching types
   */
  const String targetUnitError(const UUID& other_data_point_type);

  static const __FlashStringHelper* value_key;
  static const __FlashStringHelper* value_key_error;
  static const __FlashStringHelper* data_point_type_key;
  static const __FlashStringHelper* data_point_type_key_error;
  static const __FlashStringHelper* data_points_key;

 private:
  static String mismatchUnitError(const UUID& expected_data_point_type,
                                  const UUID& received_data_point_type);
};

}  // namespace utils
}  // namespace inamata
