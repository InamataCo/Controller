#pragma once

#include <ArduinoJson.h>

#include <memory>

#include "managers/services.h"
#include "periphery/capabilities/led_strip.h"
#include "tasks/base_task.h"
#include "utils/color.h"

namespace bernd_box {
namespace tasks {

class SetLight : public BaseTask {
 public:
  SetLight(const JsonObjectConst& parameters, Scheduler& scheduler);
  virtual ~SetLight() = default;

  const __FlashStringHelper* getType() final;
  static const __FlashStringHelper* type();


  bool OnEnable() final;
  bool Callback() final;

 private:
  static bool registered_;
  static BaseTask* factory(const JsonObjectConst& parameters,
                                           Scheduler& scheduler);

  std::shared_ptr<periphery::capabilities::LedStrip> periphery_;
  String periphery_name_;

  utils::Color color_;
};

}  // namespace tasks
}  // namespace bernd_box
