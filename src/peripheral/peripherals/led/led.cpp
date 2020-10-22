#include "led.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace led {

Led::Led(const JsonObjectConst& parameters) {
  JsonVariantConst led_pin = parameters[led_pin_key_];

  if (!led_pin.is<unsigned int>()) {
    setInvalid(led_pin_key_error_);
    return;
  }

  data_point_type_ =
      utils::UUID(parameters[utils::ValueUnit::data_point_type_key]);
  if (!data_point_type_.isValid()) {
    setInvalid(utils::ValueUnit::data_point_type_key_error);
    return;
  }

  bool error = setup(led_pin);
  if (error) {
    setInvalid(no_channels_available_error_);
    return;
  }
}

Led::~Led() { freeResources(); }

const String& Led::getType() const { return type(); }

const String& Led::type() {
  static const String name{"LED"};
  return name;
}

void Led::setValue(utils::ValueUnit value_unit) {
  if (value_unit.data_point_type != data_point_type_) {
    Services::getServer().sendError(
        type(), value_unit.sourceUnitError(data_point_type_));
    return;
  }

  // Clamp the value as a percentage between 0 and 1
  value_unit.value = std::fmax(0, std::fmin(1, value_unit.value));

  const uint max_value = (2 << resolution_) - 1;
  ledcWrite(led_channel_, value_unit.value * max_value);
}

bool Led::setup(uint pin, uint freq, uint resolution) {
  // If the LED has already been setup, free it
  if (led_channel_ != -1 || led_pin_ != -1) {
    freeResources();
  }

  // Checks if there are any free channels remaining
  if (busy_led_channels_.all()) {
    return true;
  }

  // Find the first free channel
  for (int i = 0; i < busy_led_channels_.size(); i++) {
    if (!busy_led_channels_.test(i)) {
      led_channel_ = i;
      break;
    }
  }

  // Reserve the channel as well as saving the pin and resolution
  busy_led_channels_[led_channel_] = true;
  led_pin_ = pin;
  resolution_ = resolution;

  // Setup the channel
  ledcSetup(led_channel_, freq, resolution);

  // Attach the pin to the configured channel
  ledcAttachPin(led_pin_, led_channel_);

  return false;
}

void Led::freeResources() {
  if (led_channel_ >= 0 && led_channel_ < busy_led_channels_.size()) {
    busy_led_channels_[led_channel_] = false;
  }
  ledcDetachPin(led_pin_);
  led_pin_ = -1;
  led_channel_ = -1;
  resolution_ = -1;
}

const __FlashStringHelper* Led::led_pin_key_ = F("pin");
const __FlashStringHelper* Led::led_pin_key_error_ =
    F("Missing property: pin (unsigned int)");
const __FlashStringHelper* Led::no_channels_available_error_ =
    F("No remaining LED channels available");

std::shared_ptr<Peripheral> Led::factory(const JsonObjectConst& parameters) {
  return std::make_shared<Led>(parameters);
}

bool Led::registered_ = PeripheralFactory::registerFactory(type(), factory);

bool Led::capability_set_value_ = capabilities::SetValue::registerType(type());

std::bitset<16> Led::busy_led_channels_;

const __FlashStringHelper* Led::set_value_unit_ = F("%");

}  // namespace led
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
