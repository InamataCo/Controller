#include "analog_out.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace analog_out {

AnalogOut::AnalogOut(const JsonObjectConst& parameters) {
  // Get the pin # for the GPIO output and validate data. Invalidate on error
  JsonVariantConst pin = parameters[pin_key_];
  if (!pin.is<unsigned int>()) {
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
  if (voltage_data_point_type_.isValid() !=
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
      Services::getServer().sendError(
          type(), value_unit.sourceUnitError(voltage_data_point_type_));
      return;
    }
    max_value = 3.3;
  } else if(percent_data_point_type_.isValid()) {
    if (value_unit.data_point_type != percent_data_point_type_) {
      Services::getServer().sendError(
          type(), value_unit.sourceUnitError(percent_data_point_type_));
      return;
    }
    max_value = 1.0;
  } else {
    return;
  }

  const float clamped_value =
      std::fmax(0, std::fmin(value_unit.value, max_value));
  const float dac_value = clamped_value * 255.0 / max_value;
  dacWrite(pin_, dac_value);
}

std::shared_ptr<Peripheral> AnalogOut::factory(
    const JsonObjectConst& parameters) {
  return std::make_shared<AnalogOut>(parameters);
}

bool AnalogOut::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

bool AnalogOut::capability_set_value_ =
    capabilities::SetValue::registerType(type());

const __FlashStringHelper* AnalogOut::pin_key_ = F("pin");
const __FlashStringHelper* AnalogOut::pin_key_error_ =
    F("Missing property: pin (unsigned int)");
const __FlashStringHelper* AnalogOut::invalid_pin_error_ =
    F("Pin # not valid (only 25, 26)");

const __FlashStringHelper* AnalogOut::voltage_data_point_type_key_ =
    F("voltage_data_point_type");
const __FlashStringHelper* AnalogOut::percent_data_point_type_key_ = 
    F("percent_data_point_type");
const __FlashStringHelper* AnalogOut::data_point_type_key_error_ =
    F("Missing property: data_point_type (UUID)");

}  // namespace analog_out
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box