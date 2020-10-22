#include "get_values_task.h"

namespace bernd_box {
namespace tasks {

GetValuesTask::GetValuesTask(const JsonObjectConst& parameters,
                           Scheduler& scheduler)
    : BaseTask(scheduler) {
  // Get the UUID to later find the pointer to the peripheral object
  peripheral_uuid_ = utils::UUID(parameters[peripheral_key_]);
  if (!peripheral_uuid_.isValid()) {
    setInvalid(peripheral_key_error_);
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
  peripheral_ = std::dynamic_pointer_cast<peripheral::capabilities::GetValues>(
      peripheral);
  if (!peripheral_) {
    setInvalid(peripheral::capabilities::GetValues::invalidTypeError(
        peripheral_uuid_, peripheral));
    return;
  }
}

std::shared_ptr<peripheral::capabilities::GetValues>
GetValuesTask::getPeripheral() {
  return peripheral_;
}

const utils::UUID& GetValuesTask::getPeripheralUUID() const {
  return peripheral_uuid_;
}

void GetValuesTask::makeGetValuesJson(JsonObject& result_object) {
  // Create an array for the value units and get them from the peripheral
  JsonArray value_units_doc =
      result_object.createNestedArray(utils::ValueUnit::data_points_key);
  const std::vector<utils::ValueUnit> value_units = peripheral_->getValues();

  // Create a JSON object representation for each value unit in the array
  for (const auto& value_unit : value_units) {
    JsonObject value_unit_object = value_units_doc.createNestedObject();
    value_unit_object[utils::ValueUnit::value_key] = value_unit.value;
    value_unit_object[utils::ValueUnit::data_point_type_key] =
        value_unit.data_point_type.toString();
  }

  // Add the peripheral UUID to the result
  result_object[peripheral_key_] = peripheral_uuid_.toString();
}

const __FlashStringHelper* GetValuesTask::threshold_key_ = F("threshold");
const __FlashStringHelper* GetValuesTask::threshold_key_error_ =
    F("Missing property: threshold (int)");
const __FlashStringHelper* GetValuesTask::trigger_type_key_ = F("trigger_type");
const __FlashStringHelper* GetValuesTask::trigger_type_key_error_ =
    F("Missing property: trigger_type (string)");
const __FlashStringHelper* GetValuesTask::interval_ms_key_ = F("interval_ms");
const __FlashStringHelper* GetValuesTask::interval_ms_key_error_ =
    F("Wrong type for optional property: interval_ms (unsigned int)");
const __FlashStringHelper* GetValuesTask::duration_ms_key_ = F("duration_ms");
const __FlashStringHelper* GetValuesTask::duration_ms_key_error_ =
    F("Wrong type for optional property: duration_ms (unsigned int)");

}  // namespace tasks
}  // namespace bernd_box
