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

class GetValues {
 public:
  virtual std::vector<utils::ValueUnit> getValues() = 0;

  // Type checking
  static bool registerType(const String& type);
  static bool isSupported(const String& type);
  static const std::set<String>& getTypes();

  /**
   * Error when a peripheral can't be casted to the specific capability.
   *
   * \param uuid The UUID of the peripheral to be casted
   * \param peripheral The peripheral to be casted (to get its type)
   * \return The error message
   */
  static String invalidTypeError(const utils::UUID& uuid,
                                 std::shared_ptr<Peripheral> peripheral);

 private:
  static std::set<String>& getSupportedTypes();
};

}  // namespace capabilities
}  // namespace peripheral
}  // namespace bernd_box