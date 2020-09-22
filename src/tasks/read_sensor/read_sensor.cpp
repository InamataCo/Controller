#include "read_sensor.h"

namespace bernd_box {
namespace tasks {

ReadSensor::ReadSensor(const JsonObjectConst& parameters, Scheduler& scheduler)
    : GetValueTask(parameters, scheduler) {
  // Perform one iteration, then exit
  setIterations(1);
  enable();
}

const String& ReadSensor::getType() { return type(); }

const String& ReadSensor::type() {
  static const String name{"ReadSensor"};
  return name;
}

bool ReadSensor::OnEnable() { return true; }

bool ReadSensor::Callback() {
  DynamicJsonDocument result_doc(BB_JSON_PAYLOAD_SIZE);
  JsonObject result_object = result_doc.to<JsonObject>();

  result_object[F("value")] = getPeripheral()->getValue();
  result_object[F("peripheral_name")] = getPeripheralName().c_str();

  Services::getMqtt().send(type(), result_doc);

  return true;
}

bool ReadSensor::registered_ = TaskFactory::registerTask(type(), factory);

BaseTask* ReadSensor::factory(const JsonObjectConst& parameters,
                              Scheduler& scheduler) {
  auto read_sensor = new ReadSensor(parameters, scheduler);
  if (read_sensor->isValid()) {
    return read_sensor;
  } else {
    return nullptr;
  }
}

}  // namespace tasks
}  // namespace bernd_box
