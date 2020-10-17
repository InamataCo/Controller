#pragma once

#include <ArduinoJson.h>

#include <memory>

#include "managers/services.h"
#include "peripheral/capabilities/led_strip.h"
#include "tasks/base_task.h"
#include "utils/color.h"
#include "utils/uuid.h"

namespace bernd_box {
namespace tasks {

class SetLight : public BaseTask {
 public:
  SetLight(const JsonObjectConst& parameters, Scheduler& scheduler);
  virtual ~SetLight() = default;

  const String& getType() const final;
  static const String& type();

  bool Callback() final;

 private:
  static bool registered_;
  static BaseTask* factory(const JsonObjectConst& parameters,
                           Scheduler& scheduler);

  std::shared_ptr<peripheral::capabilities::LedStrip> peripheral_;
  UUID peripheral_uuid_;

  static const __FlashStringHelper* color_key_;
  static const __FlashStringHelper* brightness_key_;
  static const __FlashStringHelper* brightness_or_color_error_;
  static const __FlashStringHelper* red_key_;
  static const __FlashStringHelper* red_key_error_;
  static const __FlashStringHelper* green_key_;
  static const __FlashStringHelper* green_key_error_;
  static const __FlashStringHelper* blue_key_;
  static const __FlashStringHelper* blue_key_error_;
  static const __FlashStringHelper* white_key_;
  static const __FlashStringHelper* white_key_error_;

  utils::Color color_;
};

}  // namespace tasks
}  // namespace bernd_box
