#include "get_value_task.h"

namespace bernd_box {
namespace tasks {

GetValueTask::GetValueTask(const JsonObjectConst& parameters,
                           Scheduler& scheduler)
    : BaseTask(scheduler) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  // Get the name to later find the pointer to the periphery object
  JsonVariantConst periphery_name = parameters[F("periphery_name")];
  if (periphery_name.isNull() || !periphery_name.is<char*>()) {
    Services::getMqtt().sendError(
        who, F("Missing property: periphery_name (string)"));
    setInvalid();
    return;
  }

  // Search for the periphery for the given name
  auto periperhy =
      Services::getLibrary().getPeriphery(periphery_name.as<String>());
  if (!periperhy) {
    Services::getMqtt().sendError(who, String(F("Could not find periphery: ")) +
                                           periphery_name.as<String>());
    setInvalid();
    return;
  }

  // Check that the periphery supports the GetValue interface capability
  periphery_ =
      std::dynamic_pointer_cast<peripheral::capabilities::GetValue>(periperhy);
  if (!periphery_) {
    Services::getMqtt().sendError(
        who, String(F("GetValue capability not supported: ")) +
                 periphery_name.as<String>() + String(F(" is a ")) +
                 periperhy->getType());
    setInvalid();
    return;
  }

  // Save the name for sending MQTT messages later
  periphery_name_ = periphery_name.as<char*>();
}

std::shared_ptr<peripheral::capabilities::GetValue>
GetValueTask::getPeriphery() {
  return periphery_;
}

const String& GetValueTask::getPeripheryName() { return periphery_name_; }

}  // namespace tasks
}  // namespace bernd_box
