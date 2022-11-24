#include "analog_out.h"

#include "peripheral/peripheral_factory.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace analog_out {

AnalogOut::AnalogOut(const ServiceGetters& services,
                     const JsonObjectConst& parameters) {
  server_ = services.getServer();
  if (server_ == nullptr) {
    setInvalid(services.server_nullptr_error_);
    return;
  }

  // Get the pin # for the GPIO output and validate data. Invalidate on error
  int pin = toPin(parameters[pin_key_]);
  if (pin < 0) {
    setInvalid(pin_key_error_);
    return;
  }
  pin_ = pin;

  // Check if the pin is valid
  if (pin != 25 && pin != 26) {
    setInvalid(invalid_pin_error_);
    return;
  }

  // Use either voltage or percent as analog set data types
  voltage_data_point_type_ =
      utils::UUID(parameters[voltage_data_point_type_key_]);
  percent_data_point_type_ =
      utils::UUID(parameters[percent_data_point_type_key_]);
  if (voltage_data_point_type_.isValid() ==
      percent_data_point_type_.isValid()) {
    setInvalid(data_point_type_key_error_);
    return;
  }
}

const String& AnalogOut::getType() const { return type(); }

const String& AnalogOut::type() {
  static const String name{"AnalogOut"};
  return name;
}

void AnalogOut::setValue(utils::ValueUnit value_unit) {
  float max_value;
  if (voltage_data_point_type_.isValid()) {
    if (value_unit.data_point_type != voltage_data_point_type_) {
      server_->sendError(type(),
                         value_unit.sourceUnitError(voltage_data_point_type_));
      return;
    }
    max_value = 3.3;
  } else if (percent_data_point_type_.isValid()) {
    if (value_unit.data_point_type != percent_data_point_type_) {
      server_->sendError(type(),
                         value_unit.sourceUnitError(percent_data_point_type_));
      return;
    }
    max_value = 1.0;
  } else {
    return;
  }

  const float clamped_value =
      std::fmax(0, std::fmin(value_unit.value, max_value));
  const float dac_value = clamped_value * 255.0 / max_value;
#ifdef ESP32
  dacWrite(pin_, dac_value);
#else
  analogWrite(pin_, dac_value);
#endif
}

std::shared_ptr<Peripheral> AnalogOut::factory(
    const ServiceGetters& services, const JsonObjectConst& parameters) {
  return std::make_shared<AnalogOut>(services, parameters);
}

bool AnalogOut::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

bool AnalogOut::capability_set_value_ =
    capabilities::SetValue::registerType(type());

const __FlashStringHelper* AnalogOut::pin_key_ = FPSTR("pin");
const __FlashStringHelper* AnalogOut::pin_key_error_ =
    FPSTR("Missing property: pin (unsigned int)");
const __FlashStringHelper* AnalogOut::invalid_pin_error_ =
    FPSTR("Pin # not valid (only 25, 26)");

const __FlashStringHelper* AnalogOut::voltage_data_point_type_key_ =
    FPSTR("voltage_data_point_type");
const __FlashStringHelper* AnalogOut::percent_data_point_type_key_ =
    FPSTR("percent_data_point_type");
const __FlashStringHelper* AnalogOut::data_point_type_key_error_ =
    FPSTR("Missing property: data_point_type (UUID)");

}  // namespace analog_out
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata