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

  state_data_point_type_ =
      utils::UUID(parameters[state_data_point_type_key_]);
  if (!state_data_point_type_.isValid()) {
    setInvalid(state_data_point_type_key_error_);
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
  if (value_unit.data_point_type != state_data_point_type_) {
    Services::getServer().sendError(
        type(), value_unit.sourceUnitError(state_data_point_type_));
    return;
  }

  float clamped_value = std::fmax(0, std::fmin(value_unit.value, 1));
  int state = std::lround(clamped_value);

  if (state == 1) {
    pinMode(pin_, HIGH);
  } else if (state == 0) {
    pinMode(pin_, LOW);
  } else {
    return;
  }
}

const __FlashStringHelper* DigitalOut::pin_key_ = F("pin");
const __FlashStringHelper* DigitalOut::pin_key_error_ =
    F("Missing property: pin (unsigned int)");

const __FlashStringHelper* DigitalOut::state_data_point_type_key_ =
    F("state_data_point_type");
const __FlashStringHelper* DigitalOut::state_data_point_type_key_error_ =
    F("Missing property: state_data_point_type (UUID)");

std::shared_ptr<Peripheral> DigitalOut::factory(const JsonObjectConst& parameters) {
  return std::make_shared<DigitalOut>(parameters);
}

bool DigitalOut::registered_ = PeripheralFactory::registerFactory(type(), factory);

bool DigitalOut::capability_set_value_ = capabilities::SetValue::registerType(type());

}  // namespace digital_out
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box