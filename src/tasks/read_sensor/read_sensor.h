#pragma once

#include <memory>

#include "ArduinoJson.h"
#include "managers/services.h"
#include "periphery/capabilities/get_value.h"
#include "tasks/base_task.h"
#include "tasks/task_factory.h"

namespace bernd_box {
namespace tasks {

class ReadSensor : public BaseTask {
 public:
  ReadSensor(const JsonObjectConst& parameters, Scheduler& scheduler,
             BaseTask::RemoveCallback remover);
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

  std::shared_ptr<periphery::capabilities::GetValue> get_value_periphery;
};

}  // namespace tasks
}  // namespace bernd_box
