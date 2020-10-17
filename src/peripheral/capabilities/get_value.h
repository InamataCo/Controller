#pragma once

#include <Arduino.h>

#include <memory>
#include <set>

#include "peripheral/peripheral.h"
#include "types.h"
#include "utils/uuid.h"

namespace bernd_box {
namespace peripheral {
namespace capabilities {

/**
 * A capability that allows a value to be returned
 */
class GetValue {
 public:
  /**
   * Returns a float value with a unit
   *
   * \return The value and its unit
   */
  virtual ValueUnit getValue() = 0;

  // Type checking
  static bool registerType(const String& type);
  static bool isSupported(const String& type);
  static const std::set<String>& getTypes();

  static String invalidTypeError(const UUID& uuid,
                                 std::shared_ptr<Peripheral> peripheral);

 private:
  static std::set<String>& getSupportedTypes();
};

}  // namespace capabilities
}  // namespace peripheral
}  // namespace bernd_box
