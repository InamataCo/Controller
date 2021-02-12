#pragma once

#include <ArduinoJson.h>

#include <chrono>
#include <memory>
#include <set>

#include "peripheral/peripheral.h"
#include "utils/uuid.h"

namespace bernd_box {
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
   * any required configurations. The handleCalibration() function should be
   * called after the specified wait time. If an error occurs, the error member
   * will return true with a ErrorResult::isError() query. The calibration
   * completed successfully if wait is zero and no error is returned.
   * 
   * \param parameters The parameters used to start a calibration step
   * \return The time to wait and error state
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
   * Repeatedly call handleCalibration according to the wait times in the
   * returned Result object. If an error occurs, check error for
   * ErrorResult::isError(). If the calibration step completed successfully,
   * no error and zero wait duration are returned.
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
}  // namespace bernd_box
