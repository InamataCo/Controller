#pragma once

#include <Arduino.h>

#include <memory>
#include <set>

#include "peripheral/peripheral.h"
#include "utils/uuid.h"

namespace bernd_box {
namespace peripheral {
namespace capabilities {

/**
 * Capability to request that a measurement be started. The peripheral returns
 * the expected wait time until the measurement should be ready. If it is not
 * ready in time, check if it is only a temporary error.
 */
class StartMeasurement {
 public:
  struct Result {
    std::chrono::nanoseconds wait;
    ErrorResult error;
  };

  /**
   * Interface to request that a measurement be started
   *
   * \param parameters Calibration parameters (e.g., temperature, offset)
   * \return Expected time in ms until measurement is expected to be ready
   */
  virtual Result startMeasurement(const JsonVariantConst& parameters) = 0;

  /**
   * Interface to check if the measurement is ready
   *
   * If it is not ready, Result::wait specefies the expected time to be ready.
   * If an error occured, Result::error will return true on isError(). If it
   * is ready, wait will be zero and error will return false on isError().
   * 
   * \see capabilities::GetValues
   *
   * \return State of the current measurement
   */
  virtual Result handleMeasurement() = 0;

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
