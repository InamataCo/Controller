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

  const String& getType() final;
  static const String& type();

  bool OnEnable() final;
  bool Callback() final;

 private:
  static bool registered_;
  static BaseTask* factory(const JsonObjectConst& parameters,
                           Scheduler& scheduler);

  std::shared_ptr<peripheral::capabilities::SetValue> peripheral_;

  peripheral::capabilities::ValueUnit value_unit_;

  // Constructor parameter keys
  const __FlashStringHelper* peripheral_name_key_ = F("peripheral_name");
  const __FlashStringHelper* value_key_ = F("value");
  const __FlashStringHelper* unit_key_ = F("unit");
};

}  // namespace tasks
}  // namespace bernd_box
