#include "write_actuator.h"

namespace bernd_box {
namespace tasks {

WriteActuator::WriteActuator(const JsonObjectConst& parameters,
                             Scheduler& scheduler)
    : BaseTask(scheduler) {
  // Get the name to later find the pointer to the peripheral object
  JsonVariantConst peripheral_name = parameters[peripheral_name_key_];
  if (peripheral_name.isNull() || !peripheral_name.is<char*>()) {
    Services::getServer().sendError(type(), String("Missing property: ") +
                                                String(peripheral_name_key_) +
                                                " (string)");
    setInvalid();
    return;
  }

  // Search for the peripheral for the given name
  auto periperhal = Services::getPeripheralController().getPeripheral(
      peripheral_name.as<String>());
  if (!periperhal) {
    Services::getMqtt().sendError(type(),
                                  String(F("Could not find peripheral: ")) +
                                      peripheral_name.as<String>());
    setInvalid();
    return;
  }

  // Check that the peripheral supports the SetValue interface capability
  peripheral_ =
      std::dynamic_pointer_cast<peripheral::capabilities::SetValue>(periperhal);
  if (!peripheral_) {
    Services::getMqtt().sendError(
        type(), String(F("SetValue capability not supported: ")) +
                    peripheral_name.as<String>() + String(F(" is a ")) +
                    periperhal->getType());
    setInvalid();
    return;
  }

  // Get the name to later find the pointer to the peripheral object
  JsonVariantConst value = parameters[value_key_];
  if (!value.is<float>()) {
    Services::getServer().sendError(
        type(), String("Missing property: ") + String(value_key_) + " (float)");
    setInvalid();
    return;
  }
  value_ = value;

  // Perform one iteration, then exit
  setIterations(1);
  enable();
}

const String& WriteActuator::getType() { return type(); }

const String& WriteActuator::type() {
  static const String name{"WriteActuator"};
  return name;
}

bool WriteActuator::OnEnable() { return true; }

bool WriteActuator::Callback() {
  peripheral_->setValue(value_);
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