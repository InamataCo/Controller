#include "read_sensor.h"

namespace bernd_box {
namespace tasks {

ReadSensor::ReadSensor(const JsonObjectConst& parameters, Scheduler& scheduler)
    : GetValueTask(parameters, scheduler) {
  // Perform one iteration, then exit
  setIterations(1);
  enable();
}

const String& ReadSensor::getType() const { return type(); }

const String& ReadSensor::type() {
  static const String name{"ReadSensor"};
  return name;
}

bool ReadSensor::Callback() {
  DynamicJsonDocument result_doc(BB_JSON_PAYLOAD_SIZE);
  JsonObject result_object = result_doc.to<JsonObject>();

  const peripheral::capabilities::ValueUnit value_unit =
      getPeripheral()->getValue();
  result_object[value_unit.value_key] = value_unit.value;
  result_object[value_unit.unit_key] = value_unit.unit.c_str();
  result_object[peripheral_uuid_key_] = getPeripheralUUID().toString();

  Services::getServer().send(type(), result_doc);

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
