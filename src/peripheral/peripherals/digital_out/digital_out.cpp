#include "digital_out.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace digital_out {

DigitalOut::DigitalOut(const JsonObjectConst& parameters) {
  // Get the pin # for the GPIO output and validate data. Invalidate on error
  JsonVariantConst pin = parameters[pin_key_];
  if (!pin.is<unsigned int>()) {
    setInvalid(pin_key_error_);
    return;
  }
  pin_ = pin;

  data_point_type_ = utils::UUID(parameters[data_point_type_key_]);
  if (!data_point_type_.isValid()) {
    setInvalid(data_point_type_key_error_);
    return;
  }

  // Setup pin to be control the GPIO state
  pinMode(pin_, OUTPUT);
}

const String& DigitalOut::getType() const { return type(); }

const String& DigitalOut::type() {
  static const String name{"DigitalOut"};
  return name;
}

void DigitalOut::setValue(utils::ValueUnit value_unit) {
  if (value_unit.data_point_type != data_point_type_) {
    Services::getServer().sendError(
        type(), value_unit.sourceUnitError(data_point_type_));
    return;
  }

  // Limit the value between 0 and 1 and then round to the nearest integer.
  // Finally, set the pin value
  float clamped_value = std::fmax(0, std::fmin(value_unit.value, 1));
  bool state = std::lround(clamped_value);
  digitalWrite(pin_, state);
}

const __FlashStringHelper* DigitalOut::pin_key_ = F("pin");
const __FlashStringHelper* DigitalOut::pin_key_error_ =
    F("Missing property: pin (unsigned int)");

std::shared_ptr<Peripheral> DigitalOut::factory(
    const JsonObjectConst& parameters) {
  return std::make_shared<DigitalOut>(parameters);
}

bool DigitalOut::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

bool DigitalOut::capability_set_value_ =
    capabilities::SetValue::registerType(type());

}  // namespace digital_out
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box