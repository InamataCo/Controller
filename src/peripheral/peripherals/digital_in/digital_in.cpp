#include "digital_in.h"

#include "peripheral/peripheral_factory.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace digital_in {

DigitalIn::DigitalIn(const JsonObjectConst& parameters) {
  // Get the pin # for the GPIO output and validate data. Invalidate on error
  int pin = toPin(parameters[pin_key_]);
  if (pin < 0) {
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
  if (!input_type.is<const char*>()) {
    setInvalid(input_type_key_error_);
    return;
  }
  if (input_type == input_type_floating) {
    pinMode(pin_, INPUT);
  } else if (input_type == input_type_pullup) {
    pinMode(pin_, INPUT_PULLUP);
  } else if (input_type == input_type_pulldown) {
#ifdef ESP32
    pinMode(pin_, INPUT_PULLDOWN);
#else
    pinMode(pin_, INPUT_PULLDOWN_16);
#endif
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
    const ServiceGetters& services, const JsonObjectConst& parameters) {
  return std::make_shared<DigitalIn>(parameters);
}

bool DigitalIn::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

bool DigitalIn::capability_get_values_ =
    capabilities::GetValues::registerType(type());

const __FlashStringHelper* DigitalIn::pin_key_ = FPSTR("pin");
const __FlashStringHelper* DigitalIn::pin_key_error_ =
    FPSTR("Missing property: pin (unsigned int)");

const __FlashStringHelper* DigitalIn::input_type_key_ = FPSTR("input_type");
const __FlashStringHelper* DigitalIn::input_type_key_error_ =
    FPSTR("Missing property: input_type (str)");
const __FlashStringHelper* DigitalIn::input_type_floating = FPSTR("floating");
const __FlashStringHelper* DigitalIn::input_type_pullup = FPSTR("pullup");
const __FlashStringHelper* DigitalIn::input_type_pulldown = FPSTR("pulldown");

}  // namespace digital_in
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata
