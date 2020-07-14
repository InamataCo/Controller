#include "poll_sensor.h"

namespace bernd_box {
namespace tasks {

PollSensor::PollSensor(const JsonObjectConst& parameters, Scheduler& scheduler)
    : GetValueTask(parameters, scheduler) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  // Get the interval with which to poll the sensor
  JsonVariantConst interval_ms = parameters[F("interval_ms")];
  if (!interval_ms.is<unsigned int>()) {
    Services::getMqtt().sendError(
        who, F("Missing property: interval_ms (unsigned int)"));
    setInvalid();
    return;
  }

  setInterval(interval_ms);

  // Optionally get the duration for which to poll the sensor [default: forever]
  JsonVariantConst duration_ms = parameters[F("duration_ms")];
  if (duration_ms.isNull()) {
    setIterations(TASK_FOREVER);
  } else if (duration_ms.is<unsigned int>()) {
    setIterations(duration_ms.as<unsigned int>() / getInterval());
  } else {
    Services::getMqtt().sendError(
        who, F("Wrong type for optional property: duration_ms (unsigned int)"));
    setInvalid();
    return;
  }

  enable();
}

const __FlashStringHelper* PollSensor::getType() { return type(); }

const __FlashStringHelper* PollSensor::type() { return F("PollSensor"); }

bool PollSensor::OnEnable() { return true; }

bool PollSensor::Callback() {
  DynamicJsonDocument result_doc(BB_MQTT_JSON_PAYLOAD_SIZE);
  JsonObject result_object = result_doc.to<JsonObject>();

  result_object[F("value")] = getPeripheral()->getValue();
  result_object[F("peripheral_name")] = getPeripheralName().c_str();

  Services::getMqtt().send(type(), result_doc);

  return true;
}

}  // namespace tasks
}  // namespace bernd_box
