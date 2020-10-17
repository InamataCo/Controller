#pragma once

#include <Arduino.h>

namespace bernd_box {
namespace peripheral {
namespace capabilities {

struct ValueUnit {
  float value;
  String unit;

  /**
   * Error for mismatching units when ValueUnit has the wrong unit
   * 
   * \param other_unit The correct unit
   * \return Error string for mismatching types
   */
  const String sourceUnitError(const String& other_unit);

  /**
   * Error for mismatching units when ValueUnit has the correct unit
   * 
   * \param other_unit The wrong unit
   * \return Error string for mismatching types
   */
  const String targetUnitError(const String& other_unit);

  static const __FlashStringHelper* value_key;
  static const __FlashStringHelper* value_key_error;
  static const __FlashStringHelper* unit_key;
  static const __FlashStringHelper* unit_key_error;

 private:
  static String mismatchUnitError(const String& expected_unit, const String& received_unit);
};

}  // namespace capabilities
}  // namespace peripheral
}  // namespace bernd_box
