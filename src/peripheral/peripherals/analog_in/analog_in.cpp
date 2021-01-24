#include "analog_in.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace analog_in {

AnalogIn::AnalogIn(const JsonObjectConst& parameters) {
  // Get the pin # for the GPIO output and validate data. Invalidate on error
  JsonVariantConst pin = parameters[pin_key_];
  if (!pin.is<unsigned int>()) {
    setInvalid(pin_key_error_);
    return;
  }
  pin_ = pin;

  if (std::find(valid_pins_.begin(), valid_pins_.end(), pin_) ==
      valid_pins_.end()) {
    setInvalid(invalid_pin_error_);
    return;
  }

  // Use both or either voltage and percent as analog readings
  voltage_data_point_type_ =
      utils::UUID(parameters[voltage_data_point_type_key_]);
  percent_data_point_type_ =
      utils::UUID(parameters[percent_data_point_type_key_]);
  if (!voltage_data_point_type_.isValid() &&
      !percent_data_point_type_.isValid()) {
    setInvalid(data_point_type_key_error_);
    return;
  }
}

const String& AnalogIn::getType() const { return type(); }

const String& AnalogIn::type() {
  static const String name{"AnalogIn"};
  return name;
}

std::vector<utils::ValueUnit> AnalogIn::getValues() {
  std::vector<utils::ValueUnit> values;
  const uint16_t value = analogRead(pin_);

  if (voltage_data_point_type_.isValid()) {
    const float voltage = value * 3.3 / 4096.0;
    values.push_back({utils::ValueUnit{
        .value = voltage, .data_point_type = voltage_data_point_type_}});
  }
  if (percent_data_point_type_.isValid()) {
    const float percent = value / 4096.0;
    values.push_back({utils::ValueUnit{
        .value = percent, .data_point_type = percent_data_point_type_}});
  }

  return values;
}

std::shared_ptr<Peripheral> AnalogIn::factory(
    const JsonObjectConst& parameters) {
  return std::make_shared<AnalogIn>(parameters);
}

bool AnalogIn::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

bool AnalogIn::capability_get_values_ =
    capabilities::GetValues::registerType(type());

const std::array<uint8_t, 8> AnalogIn::valid_pins_ = {
    32, 33, 34, 35, 36, 37, 38, 39,
};
const __FlashStringHelper* AnalogIn::pin_key_ = F("pin");
const __FlashStringHelper* AnalogIn::pin_key_error_ =
    F("Missing property: pin (unsigned int)");
const __FlashStringHelper* AnalogIn::invalid_pin_error_ =
    F("Pin # not valid (only ADC1: 32 - 39)");

const __FlashStringHelper* AnalogIn::voltage_data_point_type_key_ =
    F("voltage_data_point_type");
const __FlashStringHelper* AnalogIn::percent_data_point_type_key_ =
    F("percent_data_point_type");
const __FlashStringHelper* AnalogIn::data_point_type_key_error_ =
    F("Missing property: voltage_data_point_type (UUID) or "
      "percent_data_point_type (UUID)");

}  // namespace analog_in
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box