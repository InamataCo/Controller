#include "set_light.h"

namespace bernd_box {
namespace tasks {

SetLight::SetLight(const JsonObjectConst& parameters, Scheduler& scheduler)
    : BaseTask(scheduler) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  // Get the name to later find the pointer to the periphery object
  JsonVariantConst periphery_name = parameters[F("periphery_name")];
  if (periphery_name.isNull() || !periphery_name.is<char*>()) {
    Services::getMqtt().sendError(
        who, F("Missing property: periphery_name (string)"));
    setInvalid();
    return;
  }

  // Search for the periphery for the given name
  auto periperhy =
      Services::getLibrary().getPeriphery(periphery_name.as<String>());
  if (!periperhy) {
    Services::getMqtt().sendError(who, String(F("Could not find periphery: ")) +
                                           periphery_name.as<String>());
    setInvalid();
    return;
  }

  // Check that the periphery supports the GetValue interface capability
  periphery_ =
      std::dynamic_pointer_cast<peripheral::capabilities::LedStrip>(periperhy);
  if (!periphery_) {
    Services::getMqtt().sendError(
        who, String(F("LedStrip capability not supported: ")) +
                 periphery_name.as<String>() + String(F(" is a ")) +
                 periperhy->getType());
    setInvalid();
    return;
  }

  // The color consists of red, green , blue and optionally white components
  JsonVariantConst color = parameters[F("color")];
  JsonVariantConst brightness = parameters[F("brightness")];

  if (color.is<JsonObject>() == brightness.is<float>()) {
    Services::getMqtt().sendError(
        who, F("Either set brightness (float) or color (object)"));
    setInvalid();
    return;
  }

  if (brightness.is<float>() && brightness < 0 && brightness > 1) {
    Services::getMqtt().sendError(who, F("Brighness must be between 0 and 1"));
    setInvalid();
    return;
  } else {
    color_ = utils::Color::fromBrightness(brightness);
  }

  if (color.is<JsonObject>()) {
    JsonVariantConst color_red = color[F("red")];
    if (!color_red.is<uint8_t>()) {
      Services::getMqtt().sendError(
          who, F("Missing property: color.red (unsigned uint8_t)"));
      setInvalid();
      return;
    }

    JsonVariantConst color_green = color[F("green")];
    if (!color_green.is<uint8_t>()) {
      Services::getMqtt().sendError(
          who, F("Missing property: color.green (unsigned uint8_t)"));
      setInvalid();
      return;
    }

    JsonVariantConst color_blue = color[F("blue")];
    if (!color_blue.is<uint8_t>()) {
      Services::getMqtt().sendError(
          who, F("Missing property: color.blue (unsigned uint8_t)"));
      setInvalid();
      return;
    }

    JsonVariantConst color_white = color[F("white")];
    if (color_white.is<uint8_t>()) {
      color_ = utils::Color::fromRgbw(color_red, color_green, color_blue,
                                      color_white);
    } else if (color_white.isNull()) {
      color_ = utils::Color::fromRgbw(color_red, color_green, color_blue);
    } else {
      Services::getMqtt().sendError(
          who, F("Invalid optional property: color.white (unsigned uint8_t)"));
      setInvalid();
      return;
    }
  }

  // Save the name for sending MQTT messages later
  periphery_name_ = periphery_name.as<char*>();

  setIterations(1);
}

const __FlashStringHelper* SetLight::getType() { return type(); }

const __FlashStringHelper* SetLight::type() { return F("SetLight"); }

bool SetLight::OnEnable() {
  if (!periphery_) {
    Services::getMqtt().sendError(__PRETTY_FUNCTION__,
                                  "Selected periphery is nullptr");
    return false;
  }
  Serial.print("color: ");
  Serial.println(color_);

  periphery_->turnOn(color_);
  return true;
}

bool SetLight::Callback() { return true; }

bool SetLight::registered_ = TaskFactory::registerTask(type(), factory);

BaseTask* SetLight::factory(const JsonObjectConst& parameters,
                                            Scheduler& scheduler) {
  auto alert_sensor = new SetLight(parameters, scheduler);
  if (alert_sensor->isValid()) {
    return alert_sensor;
  } else {
    return nullptr;
  }
}

}  // namespace tasks
}  // namespace bernd_box
