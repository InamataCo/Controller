#pragma once

#include <ArduinoJson.h>

#include <memory>

#include "managers/services.h"
#include "peripheral/capabilities/start_measurement.h"
#include "tasks/get_values_task/get_values_task.h"

namespace bernd_box {
namespace tasks {
namespace read_sensor {

/**
 * Read a single value from a sensor and return it via MQTT
 */
class ReadSensor : public get_values_task::GetValuesTask {
 public:
  ReadSensor(const JsonObjectConst& parameters, Scheduler& scheduler);
  virtual ~ReadSensor() = default;

  const String& getType() const final;
  static const String& type();

  bool TaskCallback() final;

 private:
  static bool registered_;
  static BaseTask* factory(const JsonObjectConst& parameters,
                           Scheduler& scheduler);
  std::shared_ptr<peripheral::capabilities::StartMeasurement>
      start_measurement_peripheral_ = nullptr;
};

}  // namespace read_sensor
}  // namespace tasks
}  // namespace bernd_box
