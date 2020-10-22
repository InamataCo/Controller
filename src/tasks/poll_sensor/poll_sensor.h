#pragma once

#include <memory>

#include "ArduinoJson.h"
#include "tasks/get_values_task/get_values_task.h"

namespace bernd_box {
namespace tasks {

class PollSensor : public GetValuesTask {
 public:
  PollSensor(const JsonObjectConst& parameters, Scheduler& scheduler);
  virtual ~PollSensor() = default;

  const String& getType() const final;
  static const String& type();

  bool Callback() final;

 private:
  static bool registered_;
  static BaseTask* factory(const JsonObjectConst& parameters,
                           Scheduler& scheduler);
};

}  // namespace tasks
}  // namespace bernd_box
