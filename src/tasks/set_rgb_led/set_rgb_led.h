#pragma once

#include <ArduinoJson.h>

#include <memory>

#include "managers/service_getters.h"
#include "peripheral/capabilities/led_strip.h"
#include "tasks/base_task.h"
#include "utils/color.h"
#include "utils/uuid.h"

namespace inamata {
namespace tasks {
namespace set_rgb_led {

class SetRgbLed : public BaseTask {
 public:
  SetRgbLed(const JsonObjectConst& parameters, Scheduler& scheduler);
  virtual ~SetRgbLed() = default;

  const String& getType() const final;
  static const String& type();

  bool TaskCallback() final;

 private:
  static bool registered_;
  static BaseTask* factory(const ServiceGetters& services,
                           const JsonObjectConst& parameters,
                           Scheduler& scheduler);

  std::shared_ptr<peripheral::capabilities::LedStrip> peripheral_;
  utils::UUID peripheral_uuid_;

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

}  // namespace set_rgb_led
}  // namespace tasks
}  // namespace inamata
