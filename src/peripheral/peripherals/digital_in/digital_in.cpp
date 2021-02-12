#include "digital_in.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace digital_in {

DigitalIn::DigitalIn(const JsonObjectConst& parameters) {
  // Get the pin # for the GPIO output and validate data. Invalidate on error
  JsonVariantConst pin = parameters[pin_key_];
  if (!pin.is<unsigned int>()) {
    setInvalid(pin_key_error_);
    return;
  }
  pin_ = pin;

  // Get the data point type for setting the pin state
  data_point_type_ = utils::UUID(parameters[data_point_type_key_]);
  if (!data_point_type_.isValid()) {
    setInvalid(data_point_type_key_error_);
    return;
  }

  // Set up the input pin
  JsonVariantConst input_type = parameters[input_type_key_];
  if (input_type.isNull() || !input_type.is<char*>()) {
    setInvalid(input_type_key_error_);
    return;
  }
  if (input_type == input_type_floating) {
    pinMode(pin_, INPUT);
  } else if (input_type == input_type_pullup) {
    pinMode(pin_, INPUT_PULLUP);
  } else if (input_type == input_type_pulldown) {
    pinMode(pin_, INPUT_PULLDOWN);
  } else {
    setInvalid(input_type_key_error_);
    return;
  }
}

const String& DigitalIn::getType() const { return type(); }

const String& DigitalIn::type() {
  static const String name{"DigitalIn"};
  return name;
}

capabilities::GetValues::Result DigitalIn::getValues() {
  return {.values = {
              utils::ValueUnit{.value = static_cast<float>(digitalRead(pin_)),
                               .data_point_type = data_point_type_}}};
}

std::shared_ptr<Peripheral> DigitalIn::factory(
    const JsonObjectConst& parameters) {
  return std::make_shared<DigitalIn>(parameters);
}

bool DigitalIn::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

bool DigitalIn::capability_get_values_ =
    capabilities::GetValues::registerType(type());

const __FlashStringHelper* DigitalIn::pin_key_ = F("pin");
const __FlashStringHelper* DigitalIn::pin_key_error_ =
    F("Missing property: pin (unsigned int)");

const __FlashStringHelper* DigitalIn::input_type_key_ = F("input_type");
const __FlashStringHelper* DigitalIn::input_type_key_error_ =
    F("Missing property: input_type (str)");
const __FlashStringHelper* DigitalIn::input_type_floating = F("floating");
const __FlashStringHelper* DigitalIn::input_type_pullup = F("pullup");
const __FlashStringHelper* DigitalIn::input_type_pulldown = F("pulldown");

}  // namespace digital_in
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box