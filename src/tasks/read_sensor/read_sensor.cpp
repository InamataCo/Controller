#include "read_sensor.h"

#include "tasks/task_factory.h"

namespace inamata {
namespace tasks {
namespace read_sensor {

ReadSensor::ReadSensor(const ServiceGetters& services,
                       const JsonObjectConst& parameters, Scheduler& scheduler)
    : GetValuesTask(parameters, scheduler) {
  if (!isValid()) {
    return;
  }

  web_socket_ = services.getWebSocket();
  if (web_socket_ == nullptr) {
    setInvalid(services.web_socket_nullptr_error_);
    return;
  }

  // Check if the peripheral supports the startMeasurement capability. Start a
  // measurement if yes. Wait the returned amount of time to check the
  // measurement state. If doesn't support it, enable the task without delay.
  start_measurement_peripheral_ =
      std::dynamic_pointer_cast<peripheral::capabilities::StartMeasurement>(
          getPeripheral());
  if (start_measurement_peripheral_) {
    // Repeatedly run task to call handleMeasurement
    Task::setIterations(-1);

    auto result = start_measurement_peripheral_->startMeasurement(parameters);
    if (result.error.isError()) {
      setInvalid(result.error.toString());
      return;
    }
    enableDelayed(
        std::chrono::duration_cast<std::chrono::milliseconds>(result.wait)
            .count());
  } else {
    enable();
  }
}

const String& ReadSensor::getType() const { return type(); }

const String& ReadSensor::type() {
  static const String name{"ReadSensor"};
  return name;
}

bool ReadSensor::TaskCallback() {
  // If using a startMeasurement peripheral, handle the measurement. Delay
  // reading values if the result includes a wait duration. Otherwise, read the
  // values and send them to the server.
  if (start_measurement_peripheral_) {
    auto result = start_measurement_peripheral_->handleMeasurement();
    if (result.error.isError()) {
      setInvalid(result.error.toString());
      return false;
    }
    if (result.wait.count() != 0) {
      Task::delay(
          std::chrono::duration_cast<std::chrono::milliseconds>(result.wait)
              .count());
      return true;
    }
  }

  // Create the JSON doc
  doc_out.clear();
  JsonObject result_object = doc_out.to<JsonObject>();

  // Insert the value units and peripheral UUID
  ErrorResult error = packageValues(result_object);
  if (error.isError()) {
    setInvalid(error.toString());
    return false;
  }

  // Send the result to the server
  web_socket_->sendTelemetry(getTaskID(), result_object);
  return false;
}

bool ReadSensor::registered_ = TaskFactory::registerTask(type(), factory);

BaseTask* ReadSensor::factory(const ServiceGetters& services,
                              const JsonObjectConst& parameters,
                              Scheduler& scheduler) {
  return new ReadSensor(services, parameters, scheduler);
}

}  // namespace read_sensor
}  // namespace tasks
}  // namespace inamata
