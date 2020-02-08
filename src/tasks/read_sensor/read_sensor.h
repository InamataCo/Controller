#pragma once

#include <memory>

#include "ArduinoJson.h"
#include "managers/services.h"
#include "tasks/get_value_task/get_value_task.h"
#include "tasks/task_factory.h"

namespace bernd_box {
namespace tasks {

/**
 * Read a single value from a sensor and return it via MQTT
 */
class ReadSensor : public GetValueTask {
 public:
  ReadSensor(const JsonObjectConst& parameters, Scheduler& scheduler,
             BaseTask::RemoveCallback remove_callback);
  virtual ~ReadSensor() = default;

  const __FlashStringHelper* getType() final;
  static const __FlashStringHelper* type();

  bool OnEnable() final;
  bool Callback() final;

 private:
  static bool registered_;
  static std::unique_ptr<BaseTask> factory(
      const JsonObjectConst& parameters, Scheduler& scheduler,
      BaseTask::RemoveCallback remove_callback);
};

}  // namespace tasks
}  // namespace bernd_box
