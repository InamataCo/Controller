#include "digital_out.h"

#include "peripheral/peripheral_factory.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace digital_out {

DigitalOut::DigitalOut(const ServiceGetters& services,
                       const JsonObjectConst& parameters) {
  server_ = services.getServer();
  if (server_ == nullptr) {
    setInvalid(ServiceGetters::server_nullptr_error_);
    return;
  }

  // Get the pin # for the GPIO output and validate data. Invalidate on error
  int pin = toPin(parameters[pin_key_]);
  if (pin < 0) {
    setInvalid(pin_key_error_);
    return;
  }
  pin_ = pin;

  data_point_type_ = utils::UUID(parameters[data_point_type_key_]);
  if (!data_point_type_.isValid()) {
    setInvalid(data_point_type_key_error_);
    return;
  }

  // Check if the initial state was set, if yes, if it has the correct type
  JsonVariantConst initial_state = parameters[initial_state_key_];
  if (initial_state.is<bool>() == initial_state.isNull()) {
    setInvalid(initial_state_key_error_);
    return;
  }

  // Set if it is an active low peripheral. Default is active high
  JsonVariantConst active_low = parameters[active_low_key_];
  if (active_low.is<bool>() == active_low.isNull()) {
    setInvalid(active_low_key_error_);
    return;
  }
  if (active_low.as<bool>()) {
    active_low_ = true;
  }

  // Setup pin to be control the GPIO state
  pinMode(pin_, OUTPUT);
  // If specified set the initial state
  if (!initial_state.isNull()) {
    // Respect active low logic. If true, invert state
    if (active_low_) {
      digitalWrite(pin_, !initial_state.as<bool>());
    } else {
      digitalWrite(pin_, initial_state.as<bool>());
    }
  }
}

const String& DigitalOut::getType() const { return type(); }

const String& DigitalOut::type() {
  static const String name{"DigitalOut"};
  return name;
}

void DigitalOut::setValue(utils::ValueUnit value_unit) {
  if (value_unit.data_point_type != data_point_type_) {
    server_->sendError(type(), value_unit.sourceUnitError(data_point_type_));
    return;
  }

  // Limit the value between 0 and 1 and then round to the nearest integer.
  // Finally, set the pin value
  float clamped_value = std::fmax(0, std::fmin(value_unit.value, 1));
  bool state = std::lround(clamped_value);

  // Respect active low logic. If true, invert state
  if (active_low_) {
    digitalWrite(pin_, !state);
  } else {
    digitalWrite(pin_, state);
  }
}

std::shared_ptr<Peripheral> DigitalOut::factory(
    const ServiceGetters& services, const JsonObjectConst& parameters) {
  return std::make_shared<DigitalOut>(services, parameters);
}

bool DigitalOut::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

bool DigitalOut::capability_set_value_ =
    capabilities::SetValue::registerType(type());

const __FlashStringHelper* DigitalOut::pin_key_ = FPSTR("pin");
const __FlashStringHelper* DigitalOut::pin_key_error_ =
    FPSTR("Missing property: pin (unsigned int)");

const __FlashStringHelper* DigitalOut::initial_state_key_ = FPSTR("initial_state");
const __FlashStringHelper* DigitalOut::initial_state_key_error_ =
    FPSTR("Wrong property: initial_state (bool)");

const __FlashStringHelper* DigitalOut::active_low_key_ = FPSTR("active_low");
const __FlashStringHelper* DigitalOut::active_low_key_error_ =
    FPSTR("Wrong property: active_low (bool)");

}  // namespace digital_out
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata