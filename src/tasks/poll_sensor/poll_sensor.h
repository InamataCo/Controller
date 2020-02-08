#pragma once

#include <memory>

#include "ArduinoJson.h"
#include "periphery/capabilities/get_value.h"
#include "tasks/get_value_task/get_value_task.h"

namespace bernd_box {
namespace tasks {

class PollSensor : public GetValueTask {
 public:
  PollSensor(const JsonObjectConst& parameters, Scheduler& scheduler);
  virtual ~PollSensor() = default;

  const __FlashStringHelper* getType() final;
  static const __FlashStringHelper* type();

  bool OnEnable() final;
  bool Callback() final;

 private:
  static bool registered_;
  static std::unique_ptr<BaseTask> factory(
      const JsonObjectConst& parameters, Scheduler& scheduler);
};

}  // namespace tasks
}  // namespace bernd_box
