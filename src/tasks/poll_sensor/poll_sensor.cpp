#include "poll_sensor.h"

namespace bernd_box {
namespace tasks {

PollSensor::PollSensor(const JsonObjectConst& parameters, Scheduler& scheduler)
    : GetValueTask(parameters, scheduler) {
  // Get the interval with which to poll the sensor
  JsonVariantConst interval_ms = parameters[interval_ms_key_];
  if (!interval_ms.is<unsigned int>()) {
    setInvalid(interval_ms_key_error_);
    return;
  }

  setInterval(interval_ms);

  // Optionally get the duration for which to poll the sensor [default: forever]
  JsonVariantConst duration_ms = parameters[duration_ms_key_];
  if (duration_ms.isNull()) {
    setIterations(TASK_FOREVER);
  } else if (duration_ms.is<unsigned int>()) {
    setIterations(duration_ms.as<unsigned int>() / getInterval());
  } else {
    setInvalid(duration_ms_key_error_);
    return;
  }

  enable();
}

const String& PollSensor::getType() const { return type(); }

const String& PollSensor::type() {
  static const String name{"PollSensor"};
  return name;
}

bool PollSensor::Callback() {
  DynamicJsonDocument result_doc(BB_JSON_PAYLOAD_SIZE);
  JsonObject result_object = result_doc.to<JsonObject>();

  const peripheral::capabilities::ValueUnit value_unit =
      getPeripheral()->getValue();
  result_object[value_unit.value_key] = value_unit.value;
  result_object[value_unit.unit_key] = value_unit.unit.c_str();
  result_object[peripheral_uuid_key_] = getPeripheralUUID().toString();

  Services::getMqtt().send(type(), result_doc);

  return true;
}

}  // namespace tasks
}  // namespace bernd_box
