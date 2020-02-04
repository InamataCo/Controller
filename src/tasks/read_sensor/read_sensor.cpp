#include "read_sensor.h"

namespace bernd_box {
namespace tasks {

ReadSensor::ReadSensor(const JsonObjectConst& parameters, Scheduler& scheduler,
                       BaseTask::RemoveCallback remove_callback)
    : BaseTask(scheduler, remove_callback) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  JsonVariantConst periphery_name = parameters[F("periphery_name")];
  if (periphery_name.isNull() || !periphery_name.is<char*>()) {
    Services::getMqtt().sendError(
        who, F("Missing property: periphery_name (string)"));
    setInvalid();
    return;
  }

  auto periperhy =
      Services::getLibrary().getPeriphery(periphery_name.as<String>());
  if (!periperhy) {
    Services::getMqtt().sendError(who, String(F("Could not find periphery: ")) +
                                           periphery_name.as<String>());
    setInvalid();
    return;
  }

  if (!periphery::capabilities::GetValue::isSupported(periperhy->getType())) {
    Services::getMqtt().sendError(
        who, String(F("GetValue capability not supported: ")) +
                 periphery_name.as<String>() + String(F(" is a ")) +
                 periperhy->getType());
    setInvalid();
    return;
  }
}

const __FlashStringHelper* ReadSensor::getType() { return type(); }

const __FlashStringHelper* ReadSensor::type() { return F("ReadSensor"); }

bool ReadSensor::OnEnable() { return true; }

bool ReadSensor::Callback() { return false; }

bool ReadSensor::registered_ = TaskFactory::registerTask(type(), factory);

std::unique_ptr<BaseTask> ReadSensor::factory(
    const JsonObjectConst& parameters, Scheduler& scheduler,
    BaseTask::RemoveCallback remover) {
  auto read_sensor = std::unique_ptr<ReadSensor>(
      new ReadSensor(parameters, scheduler, remover));
  if (read_sensor->isValid()) {
    return read_sensor;
  } else {
    return std::unique_ptr<BaseTask>();
  }
}

}  // namespace tasks
}  // namespace bernd_box
