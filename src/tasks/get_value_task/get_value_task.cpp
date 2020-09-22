#include "get_value_task.h"

namespace bernd_box {
namespace tasks {

GetValueTask::GetValueTask(const JsonObjectConst& parameters,
                           Scheduler& scheduler)
    : BaseTask(scheduler) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  // Get the name to later find the pointer to the peripheral object
  JsonVariantConst peripheral_name = parameters[F("peripheral_name")];
  if (peripheral_name.isNull() || !peripheral_name.is<char*>()) {
    Services::getMqtt().sendError(
        who, F("Missing property: peripheral_name (string)"));
    setInvalid();
    return;
  }

  // Search for the peripheral for the given name
  auto periperhal = Services::getPeripheralController().getPeripheral(
      peripheral_name.as<String>());
  if (!periperhal) {
    Services::getMqtt().sendError(who,
                                  String(F("Could not find peripheral: ")) +
                                      peripheral_name.as<String>());
    setInvalid();
    return;
  }

  // Check that the peripheral supports the GetValue interface capability
  peripheral_ =
      std::dynamic_pointer_cast<peripheral::capabilities::GetValue>(periperhal);
  if (!peripheral_) {
    Services::getMqtt().sendError(
        who, String(F("GetValue capability not supported: ")) +
                 peripheral_name.as<String>() + String(F(" is a ")) +
                 periperhal->getType());
    setInvalid();
    return;
  }

  // Save the name for sending MQTT messages later
  peripheral_name_ = peripheral_name.as<char*>();
}

std::shared_ptr<peripheral::capabilities::GetValue>
GetValueTask::getPeripheral() {
  return peripheral_;
}

const String& GetValueTask::getPeripheralName() { return peripheral_name_; }

}  // namespace tasks
}  // namespace bernd_box
