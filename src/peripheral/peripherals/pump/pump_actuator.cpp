#include "pump_actuator.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace pump {

PumpActuator::PumpActuator(const JsonObjectConst& parameter) {
  // Get the pin # for the pump and validate data. Invalidate on error
  JsonVariantConst pump_pin = parameter[pump_pin_key_];
  if (!pump_pin.is<unsigned int>()) {
    setInvalid(pump_pin_key_error_);
    return;
  }
  pump_pin_ = pump_pin;

  // Setup pin to be control the GPIO state
  pinMode(pump_pin_, OUTPUT);
}

const String& PumpActuator::getType() const { return type(); }

const String& PumpActuator::type() {
  static const String name{"PumpActuator"};
  return name;
}

void PumpActuator::setValue(capabilities::ValueUnit value_unit) {
  if (value_unit.unit != String(set_value_unit_)) {
    Services::getServer().sendError(
        type(), value_unit.sourceUnitError(set_value_unit_));
    return;
  }

  float limited_value = std::fmax(0, std::fmin(value_unit.value, 1));
  int state = std::lround(limited_value);

  if (state == 1) {
    pinMode(pump_pin_, HIGH);
  } else if (state == 0) {
    pinMode(pump_pin_, LOW);
  }
}

const __FlashStringHelper* PumpActuator::set_value_unit_ = F("bool");

const __FlashStringHelper* PumpActuator::pump_pin_key_ = F("pump_pin");
const __FlashStringHelper* PumpActuator::pump_pin_key_error_ =
    F("Missing property: pump_pin (unsigned int)");

}  // namespace pump
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box