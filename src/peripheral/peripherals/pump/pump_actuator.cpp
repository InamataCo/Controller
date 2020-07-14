#include "pump_actuator.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace pump {

PumpActuator::PumpActuator(const JsonObjectConst& parameter) {
  // Get the pin # for the pump and validate data. Invalidate on error
  JsonVariantConst pump_pin = parameter[pump_pin_name_];
  if (!pump_pin.is<unsigned int>()) {
    Services::getServer().sendError(type(), String(F("Missing property: ")) +
                                                pump_pin_name_ +
                                                F(" (unsigned int)"));
    setInvalid();
    return;
  }
  pump_pin_ = pump_pin;

  // Setup pin to be control the GPIO state
  pinMode(pump_pin_, OUTPUT);
}

const String& PumpActuator::getType() { return type(); }

const String& PumpActuator::type() {
  static const String name{"PumpActuator"};
  return name;
}

void PumpActuator::setValue(float value) {
  int state = std::lround(value);

  if (state == 1) {
    pinMode(pump_pin_, HIGH);
  } else if (state == 0) {
    pinMode(pump_pin_, LOW);
  } else {
    Services::getServer().sendError(type(),
                                    String(F("Set invalid value: ")) + value);
  }
}

}  // namespace pump
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box