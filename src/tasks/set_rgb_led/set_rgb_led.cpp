#include "set_rgb_led.h"

namespace bernd_box {
namespace tasks {
namespace set_rgb_led {

SetRgbLed::SetRgbLed(const JsonObjectConst& parameters, Scheduler& scheduler)
    : BaseTask(scheduler, parameters) {
  if (!isValid()) {
    return;
  }

  // Get the UUID to later find the pointer to the peripheral object
  peripheral_uuid_ = utils::UUID(parameters[peripheral_key_]);
  if (!peripheral_uuid_.isValid()) {
    setInvalid(peripheral_key_error_);
    return;
  }

  // Search for the peripheral for the given name
  auto peripheral =
      Services::getPeripheralController().getPeripheral(peripheral_uuid_);
  if (!peripheral) {
    setInvalid(peripheralNotFoundError(peripheral_uuid_));
    return;
  }

  // Check that the peripheral supports the GetValue interface capability
  peripheral_ =
      std::dynamic_pointer_cast<peripheral::capabilities::LedStrip>(peripheral);
  if (!peripheral_) {
    setInvalid(peripheral::capabilities::LedStrip::invalidTypeError(
        peripheral_uuid_, peripheral));
    return;
  }

  // The color consists of red, green , blue and optionally white components
  JsonVariantConst color = parameters[color_key_];
  JsonVariantConst brightness = parameters[brightness_key_];

  // XOR test. Either color or brightness may be set
  if (color.is<JsonObject>() == brightness.is<float>()) {
    setInvalid(brightness_or_color_error_);
    return;
  }

  // Limit the brightness range from 0 to 1
  if (brightness.is<float>()) {
    float brightness_clipped =
        std::fmax(0, std::fmin(brightness.as<float>(), 1));
    color_ = utils::Color::fromBrightness(brightness_clipped);
  }

  if (color.is<JsonObject>()) {
    JsonVariantConst color_red = color[red_key_];
    if (!color_red.is<uint8_t>()) {
      setInvalid(red_key_error_);
      return;
    }

    JsonVariantConst color_green = color[green_key_];
    if (!color_green.is<uint8_t>()) {
      setInvalid(green_key_error_);
      return;
    }

    JsonVariantConst color_blue = color[blue_key_];
    if (!color_blue.is<uint8_t>()) {
      setInvalid(blue_key_error_);
      return;
    }

    JsonVariantConst color_white = color[white_key_];
    if (color_white.is<uint8_t>()) {
      color_ = utils::Color::fromRgbw(color_red, color_green, color_blue,
                                      color_white);
    } else if (color_white.isNull()) {
      color_ = utils::Color::fromRgbw(color_red, color_green, color_blue);
    } else {
      setInvalid(white_key_error_);
      return;
    }
  }

  enable();
}

const String& SetRgbLed::getType() const { return type(); }

const String& SetRgbLed::type() {
  static const String name{"SetRgbLed"};
  return name;
}

bool SetRgbLed::TaskCallback() {
  peripheral_->turnOn(color_);
  return false;
}

bool SetRgbLed::registered_ = TaskFactory::registerTask(type(), factory);

BaseTask* SetRgbLed::factory(const JsonObjectConst& parameters,
                             Scheduler& scheduler) {
  return new SetRgbLed(parameters, scheduler);
}

const __FlashStringHelper* SetRgbLed::color_key_ = F("color");
const __FlashStringHelper* SetRgbLed::brightness_key_ = F("brightness");
const __FlashStringHelper* SetRgbLed::brightness_or_color_error_ =
    F("Either set brightness (float) or color (object)");
const __FlashStringHelper* SetRgbLed::red_key_ = F("red");
const __FlashStringHelper* SetRgbLed::red_key_error_ =
    F("Missing property: color.red (unsigned uint8_t)");
const __FlashStringHelper* SetRgbLed::green_key_ = F("green");
const __FlashStringHelper* SetRgbLed::green_key_error_ =
    F("Missing property: color.green (unsigned uint8_t)");
const __FlashStringHelper* SetRgbLed::blue_key_ = F("blue");
const __FlashStringHelper* SetRgbLed::blue_key_error_ =
    F("Missing property: color.blue (unsigned uint8_t)");
const __FlashStringHelper* SetRgbLed::white_key_ = F("white");
const __FlashStringHelper* SetRgbLed::white_key_error_ =
    F("Invalid optional property: color.white (unsigned uint8_t)");

}  // namespace set_rgb_led
}  // namespace tasks
}  // namespace bernd_box