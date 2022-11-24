#pragma once

#include <ArduinoJson.h>

#include <chrono>
#include <memory>
#include <set>

#include "peripheral/peripheral.h"
#include "utils/uuid.h"

namespace inamata {
namespace peripheral {
namespace capabilities {

class Calibrate {
 public:
  struct Result {
    std::chrono::nanoseconds wait;
    ErrorResult error;
  };

  /**
   * Start a calibration step with the specified parameters
   * 
   * A user initiated calibration step. The parameters define which step and
   * any required configurations. The handleCalibration() function will be
   * called according to the returned delay. It will be called at least once
   * even if wait and error equal zero.
   * 
   * \param parameters The parameters used to start a calibration step
   * \return Whether the peripheral was configured correctly
   */
  virtual Result startCalibration(const JsonObjectConst& parameters) = 0;

  /**
   * Continue the calibration step
   * 
   * A calibration is broken down into multiple calibration steps. Some require
   * user input which is handled by startCalibration(const JsonObjectConst&)
   * and others can be automatically started when a previous calibration step
   * finished.
   * 
   * handleCalibration() is called repeatedly according to the wait times in
   * the returned Result object. If an error occurs, check error for
   * ErrorResult::isError(). If the calibration step completed successfully,
   * no error and zero wait duration are returned.
   * 
   * \return The time to wait until calling again and if an error occured
   */
  virtual Result handleCalibration() = 0;

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
}  // namespace inamata
