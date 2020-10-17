#include "get_value_task.h"

namespace bernd_box {
namespace tasks {

GetValueTask::GetValueTask(const JsonObjectConst& parameters,
                           Scheduler& scheduler)
    : BaseTask(scheduler) {
  // Get the UUID to later find the pointer to the peripheral object
  peripheral_uuid_ = UUID(parameters[peripheral_uuid_key_]);
  if (!peripheral_uuid_.isValid()) {
    setInvalid(peripheral_uuid_key_error_);
    return;
  }

  // Search for the peripheral for the given name
  auto peripheral =
      Services::getPeripheralController().getPeripheral(peripheral_uuid_);
  if (!peripheral) {
    setInvalid(peripheralNotFoundError(peripheral_uuid_));
    return;
  }

  // Check that the peripheral supports the GetValue interface capability
  peripheral_ =
      std::dynamic_pointer_cast<peripheral::capabilities::GetValue>(peripheral);
  if (!peripheral_) {
    setInvalid(peripheral::capabilities::GetValue::invalidTypeError(
        peripheral_uuid_, peripheral));
    return;
  }
}

std::shared_ptr<peripheral::capabilities::GetValue>
GetValueTask::getPeripheral() {
  return peripheral_;
}

const UUID& GetValueTask::getPeripheralUUID() const { return peripheral_uuid_; }

const __FlashStringHelper* GetValueTask::threshold_key_ = F("threshold");
const __FlashStringHelper* GetValueTask::threshold_key_error_ =
    F("Missing property: threshold (int)");
const __FlashStringHelper* GetValueTask::trigger_type_key_ = F("trigger_type");
const __FlashStringHelper* GetValueTask::trigger_type_key_error_ =
    F("Missing property: trigger_type (string)");
const __FlashStringHelper* GetValueTask::interval_ms_key_ = F("interval_ms");
const __FlashStringHelper* GetValueTask::interval_ms_key_error_ =
    F("Wrong type for optional property: interval_ms (unsigned int)");
const __FlashStringHelper* GetValueTask::duration_ms_key_ = F("duration_ms");
const __FlashStringHelper* GetValueTask::duration_ms_key_error_ =
    F("Wrong type for optional property: duration_ms (unsigned int)");

}  // namespace tasks
}  // namespace bernd_box
