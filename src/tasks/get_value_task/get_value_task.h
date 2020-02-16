#pragma once

#include <ArduinoJson.h>

#include <memory>

#include "managers/services.h"
#include "peripheral/capabilities/get_value.h"
#include "tasks/base_task.h"

namespace bernd_box {
namespace tasks {

/**
 * Abstract class that implements getting a periphery for a given name from a
 * JSON object via the MQTT interface.
 */
class GetValueTask : public BaseTask {
 public:
  GetValueTask(const JsonObjectConst& parameters, Scheduler& scheduler);
  virtual ~GetValueTask() = default;

  std::shared_ptr<peripheral::capabilities::GetValue> getPeriphery();
  const String& getPeripheryName();

 private:
  std::shared_ptr<peripheral::capabilities::GetValue> periphery_;
  String periphery_name_;
};

}  // namespace tasks
}  // namespace bernd_box
