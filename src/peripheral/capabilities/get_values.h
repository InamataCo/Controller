#pragma once

#include <Arduino.h>

#include <memory>
#include <set>
#include <vector>

#include "peripheral/peripheral.h"
#include "utils/uuid.h"
#include "utils/value_unit.h"

namespace inamata {
namespace peripheral {
namespace capabilities {

class GetValues {
 public:
  struct Result {
    std::vector<utils::ValueUnit> values;
    ErrorResult error;
  };

  /**
   * Interface to get ValueUnits
   *
   * \return On success, a vector with ValueUnits. On error, true for isError()
   */
  virtual Result getValues() = 0;

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

 protected:
  static const __FlashStringHelper* get_values_error_;

 private:
  static std::set<String>& getSupportedTypes();
};

}  // namespace capabilities
}  // namespace peripheral
}  // namespace inamata