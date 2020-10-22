#include "pump_actuator.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace pump {

PumpActuator::PumpActuator(const JsonObjectConst& parameters) {
  // Get the pin # for the pump and validate data. Invalidate on error
  JsonVariantConst pump_pin = parameters[pump_pin_key_];
  if (!pump_pin.is<unsigned int>()) {
    setInvalid(pump_pin_key_error_);
    return;
  }
  pump_pin_ = pump_pin;

  pump_state_data_point_type_ =
      utils::UUID(parameters[pump_state_data_point_type_key_]);
  if (!pump_state_data_point_type_.isValid()) {
    setInvalid(pump_state_data_point_type_key_error_);
    return;
  }

  // Setup pin to be control the GPIO state
  pinMode(pump_pin_, OUTPUT);
}

const String& PumpActuator::getType() const { return type(); }

const String& PumpActuator::type() {
  static const String name{"PumpActuator"};
  return name;
}

void PumpActuator::setValue(utils::ValueUnit value_unit) {
  if (value_unit.data_point_type != pump_state_data_point_type_) {
    Services::getServer().sendError(
        type(), value_unit.sourceUnitError(pump_state_data_point_type_));
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

const __FlashStringHelper* PumpActuator::pump_pin_key_ = F("pump_pin");
const __FlashStringHelper* PumpActuator::pump_pin_key_error_ =
    F("Missing property: pump_pin (unsigned int)");

const __FlashStringHelper* PumpActuator::pump_state_data_point_type_key_ =
    F("pump_state_data_point_type");
const __FlashStringHelper* PumpActuator::pump_state_data_point_type_key_error_ =
    F("Missing property: pump_state_data_point_type (UUID)");

}  // namespace pump
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box