#include "calibrate.h"

namespace bernd_box {
namespace tasks {
namespace calibrate {

Calibrate::Calibrate(const JsonObjectConst& parameters, Scheduler& scheduler)
    : BaseTask(scheduler, parameters) {
  // Abort if the base class failed initialization
  if (!isValid()) {
    return;
  }

  // Get the UUID to later find the pointer to the peripheral object
  utils::UUID peripheral_uuid(parameters[peripheral_key_]);
  if (!peripheral_uuid.isValid()) {
    setInvalid(peripheral_key_error_);
    return;
  }

  // Search for the peripheral for the given name
  auto peripheral =
      Services::getPeripheralController().getPeripheral(peripheral_uuid);
  if (!peripheral) {
    setInvalid(peripheral_not_found_error_);
    return;
  }

  // Check that the peripheral supports the Calibrate interface capability
  peripheral_ = std::dynamic_pointer_cast<peripheral::capabilities::Calibrate>(
      peripheral);
  if (!peripheral_) {
    setInvalid(peripheral::capabilities::Calibrate::invalidTypeError(
        peripheral_uuid, peripheral));
    return;
  }

  peripheral::capabilities::Calibrate::Result result =
      peripheral_->startCalibration(parameters);
  if (result.error.isError()) {
    setInvalid(result.error.toString());
    return;
  }

  enableDelayed(
      std::chrono::duration_cast<std::chrono::milliseconds>(result.wait)
          .count());
}

bool Calibrate::TaskCallback() {
  auto result = peripheral_->handleCalibration();
  if (result.error.isError()) {
    setInvalid(result.error.toString());
    return false;
  } else if (result.wait.count() != 0) {
    // Delays execution of next callback for the specified milliseconds
    Calibrate::delay(
        std::chrono::duration_cast<std::chrono::milliseconds>(result.wait)
            .count());
    return true;
  } else {
    return false;
  }
}

}  // namespace calibrate
}  // namespace tasks
}  // namespace bernd_box