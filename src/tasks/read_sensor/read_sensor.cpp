#include "read_sensor.h"

namespace bernd_box {
namespace tasks {

ReadSensor::ReadSensor(const JsonObjectConst& parameters, Scheduler& scheduler,
                       BaseTask::RemoveCallback remove_callback)
    : GetValueTask(parameters, scheduler, remove_callback) {
  // Perform one iteration, then exit
  setIterations(1);
  enable();
}

const __FlashStringHelper* ReadSensor::getType() { return type(); }

const __FlashStringHelper* ReadSensor::type() { return F("ReadSensor"); }

bool ReadSensor::OnEnable() { return true; }

bool ReadSensor::Callback() {
  DynamicJsonDocument result_doc(BB_MQTT_JSON_PAYLOAD_SIZE);
  JsonObject result_object = result_doc.to<JsonObject>();

  result_object[F("value")] = getPeriphery()->getValue();
  result_object[F("periphery_name")] = getPeripheryName().c_str();

  Services::getMqtt().send(type(), result_doc);

  return true;
}

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
