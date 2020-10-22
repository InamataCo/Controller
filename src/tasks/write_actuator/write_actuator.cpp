#include "write_actuator.h"

namespace bernd_box {
namespace tasks {

WriteActuator::WriteActuator(const JsonObjectConst& parameters,
                             Scheduler& scheduler)
    : BaseTask(scheduler) {
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

  // Check that the peripheral supports the SetValue interface capability
  peripheral_ =
      std::dynamic_pointer_cast<peripheral::capabilities::SetValue>(peripheral);
  if (!peripheral_) {
    setInvalid(peripheral::capabilities::SetValue::invalidTypeError(
        peripheral_uuid, peripheral));
    return;
  }

  // Get the value
  JsonVariantConst value = parameters[value_unit_.value_key];
  if (!value.is<float>()) {
    setInvalid(value_unit_.value_key_error);
    return;
  }

  // Get the unit of the value
  utils::UUID data_point_type(parameters[value_unit_.data_point_type_key]);
  if (!data_point_type.isValid()) {
    setInvalid(value_unit_.data_point_type_key_error);
    return;
  }

  // Save the ValueUnit
  value_unit_ =
      utils::ValueUnit{.value = value, .data_point_type = data_point_type};

  // Perform one iteration, then exit
  setIterations(1);
  enable();
}

const String& WriteActuator::getType() const { return type(); }

const String& WriteActuator::type() {
  static const String name{"WriteActuator"};
  return name;
}

bool WriteActuator::Callback() {
  peripheral_->setValue(value_unit_);
  return true;
}

bool WriteActuator::registered_ = TaskFactory::registerTask(type(), factory);

BaseTask* WriteActuator::factory(const JsonObjectConst& parameters,
                                 Scheduler& scheduler) {
  auto write_sensor = new WriteActuator(parameters, scheduler);
  if (write_sensor->isValid()) {
    return write_sensor;
  } else {
    return nullptr;
  }
}

}  // namespace tasks
}  // namespace bernd_box