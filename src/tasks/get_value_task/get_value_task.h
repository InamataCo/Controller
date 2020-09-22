#pragma once

#include <ArduinoJson.h>

#include <memory>

#include "managers/services.h"
#include "peripheral/capabilities/get_value.h"
#include "tasks/base_task.h"

namespace bernd_box {
namespace tasks {

/**
 * Abstract class that implements getting a peripheral which supports the
 * GetValue capability for a given name.
 */
class GetValueTask : public BaseTask {
 public:
  GetValueTask(const JsonObjectConst& parameters, Scheduler& scheduler);
  virtual ~GetValueTask() = default;

  std::shared_ptr<peripheral::capabilities::GetValue> getPeripheral();
  const String& getPeripheralName();

 private:
  std::shared_ptr<peripheral::capabilities::GetValue> peripheral_;
  String peripheral_name_;
};

}  // namespace tasks
}  // namespace bernd_box
