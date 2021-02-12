#include "poll_sensor.h"

namespace bernd_box {
namespace tasks {
namespace poll_sensor {

PollSensor::PollSensor(const JsonObjectConst& parameters, Scheduler& scheduler)
    : GetValuesTask(parameters, scheduler) {
  if (!isValid()) {
    return;
  }

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

void PollSensor::TaskCallback() {
  // Create a JSON doc on the heap
  DynamicJsonDocument result_doc(BB_JSON_PAYLOAD_SIZE);
  JsonObject result_object = result_doc.to<JsonObject>();

  // Read the peripheral's value units and its UUID and add them to the JSON doc
  ErrorResult error = makeTelemetryJson(result_object);

  // Check if the values could be successfully read
  if (error.isError()) {
    setInvalid(error.toString());
    return;
  }

  // Send the value units and peripheral UUID to the server
  Services::getServer().send(type(), result_doc);
}

bool PollSensor::registered_ = TaskFactory::registerTask(type(), factory);

BaseTask* PollSensor::factory(const JsonObjectConst& parameters,
                              Scheduler& scheduler) {
  return new PollSensor(parameters, scheduler);
}

}  // namespace poll_sensor
}  // namespace tasks
}  // namespace bernd_box
