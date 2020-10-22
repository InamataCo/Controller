#pragma once

#include <Arduino.h>

#include <memory>
#include <set>

#include "peripheral/peripheral.h"
#include "utils/uuid.h"
#include "utils/value_unit.h"

namespace bernd_box {
namespace peripheral {
namespace capabilities {

/**
 * A capability that allows a unitless value to be set
 */
class SetValue {
 public:
  /**
   * Interface to set a unit-less value
   *
   * \param value The value of the unitless value
   */
  virtual void setValue(utils::ValueUnit value_unit) = 0;

  // Type checking
  static bool registerType(const String& type);
  static bool isSupported(const String& type);
  static const std::set<String>& getTypes();

  static String invalidTypeError(const utils::UUID& uuid,
                                 std::shared_ptr<Peripheral> peripheral);

 private:
  static std::set<String>& getSupportedTypes();
};

}  // namespace capabilities
}  // namespace peripheral
}  // namespace bernd_box
