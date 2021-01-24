#pragma once

#include <ArduinoJson.h>

#include <memory>

#include "managers/services.h"
#include "peripheral/capabilities/set_value.h"
#include "tasks/base_task.h"

namespace bernd_box {
namespace tasks {
namespace set_value {

class SetValue : public BaseTask {
 public:
  SetValue(const JsonObjectConst& parameters, Scheduler& scheduler);
  virtual ~SetValue() = default;

  const String& getType() const final;
  static const String& type();

  bool Callback() final;

 private:
  static bool registered_;
  static BaseTask* factory(const JsonObjectConst& parameters,
                           Scheduler& scheduler);

  std::shared_ptr<peripheral::capabilities::SetValue> peripheral_;

  utils::ValueUnit value_unit_;
};

}  // namespace set_value
}  // namespace tasks
}  // namespace bernd_box
