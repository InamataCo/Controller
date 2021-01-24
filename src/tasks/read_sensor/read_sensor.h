#pragma once

#include <memory>

#include "ArduinoJson.h"
#include "managers/services.h"
#include "tasks/get_values_task/get_values_task.h"
#include "tasks/task_factory.h"

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

  bool Callback() final;

 private:
  static bool registered_;
  static BaseTask* factory(const JsonObjectConst& parameters,
                           Scheduler& scheduler);
};

}  // namespace read_sensor
}  // namespace tasks
}  // namespace bernd_box
