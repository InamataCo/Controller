#pragma once

#include <ArduinoJson.h>

#include <memory>

#include "managers/services.h"
#include "peripheral/capabilities/set_value.h"
#include "tasks/base_task.h"

namespace bernd_box {
namespace tasks {

class WriteActuator : public BaseTask {
 public:
  WriteActuator(const JsonObjectConst& parameters, Scheduler& scheduler);
  virtual ~WriteActuator() = default;

  const String& getType() const final;
  static const String& type();

  bool Callback() final;

 private:
  static bool registered_;
  static BaseTask* factory(const JsonObjectConst& parameters,
                           Scheduler& scheduler);

  std::shared_ptr<peripheral::capabilities::SetValue> peripheral_;

  peripheral::capabilities::ValueUnit value_unit_;
};

}  // namespace tasks
}  // namespace bernd_box
