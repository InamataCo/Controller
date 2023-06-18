#include "analog_in.h"

#include "peripheral/peripheral_factory.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace analog_in {

AnalogIn::AnalogIn(const JsonObjectConst& parameters) {
  // Get the pin # for the GPIO output and validate data. Invalidate on error
  int pin = toPin(parameters[pin_key_]);
  if (pin < 0) {
    setInvalid(pin_key_error_);
    return;
  }
  pin_ = pin;

  if (std::find(valid_pins_.begin(), valid_pins_.end(), pin_) ==
      valid_pins_.end()) {
    setInvalid(invalid_pin_error_);
    return;
  }

  // Use both or either voltage and unit as analog readings
  voltage_data_point_type_ =
      utils::UUID(parameters[voltage_data_point_type_key_]);
  percent_data_point_type_ =
      utils::UUID(parameters[percent_data_point_type_key_]);
  parseConvertToUnit(parameters);
  if (!voltage_data_point_type_.isValid() &&
      !percent_data_point_type_.isValid() && !unit_data_point_type_.isValid()) {
    setInvalid(data_point_type_key_error_);
    return;
  }
}

const String& AnalogIn::getType() const { return type(); }

const String& AnalogIn::type() {
  static const String name{"AnalogIn"};
  return name;
}

void AnalogIn::parseConvertToUnit(const JsonObjectConst& parameters) {
  // Do a linear conversion from the voltage to a different unit
  JsonVariantConst min_v = parameters[min_v_key_];
  if (!min_v.is<float>()) {
    return;
  }
  JsonVariantConst max_v = parameters[max_v_key_];
  if (!max_v.is<float>()) {
    return;
  }
  JsonVariantConst min_unit = parameters[min_unit_key_];
  if (!min_unit.is<float>()) {
    return;
  }
  JsonVariantConst max_unit = parameters[max_unit_key_];
  if (!max_unit.is<float>()) {
    return;
  }
  JsonVariantConst limit_unit = parameters[limit_unit_key_];
  if (limit_unit.is<bool>()) {
    limit_unit_ = limit_unit;
  }
  utils::UUID unit_data_point_type =
      utils::UUID(parameters[unit_data_point_type_key_]);
  if (!unit_data_point_type.isValid()) {
    return;
  }
  if (min_v == max_v) {
    setInvalid();
    return;
  }

  // Formula taken from SO:
  // https://stackoverflow.com/questions/5731863/mapping-a-numeric-range-onto-another
  v_to_unit_slope_ = 1.0 * (max_unit.as<float>() - min_unit.as<float>()) /
                     (max_v.as<float>() - min_v.as<float>());
  min_v_ = min_v;
  min_unit_ = min_unit;
  max_unit_ = max_unit;
  unit_data_point_type_ = unit_data_point_type;
}

capabilities::GetValues::Result AnalogIn::getValues() {
  std::vector<utils::ValueUnit> values;
  const uint16_t value = analogRead(pin_);
  const float voltage = value * 3.3 / 4096.0;

  if (voltage_data_point_type_.isValid()) {
    values.push_back({utils::ValueUnit{
        .value = voltage, .data_point_type = voltage_data_point_type_}});
  }
  if (percent_data_point_type_.isValid()) {
    const float percentage = value / 4096.0; 
    values.push_back({utils::ValueUnit{
      .value = percentage, .data_point_type = percent_data_point_type_
    }});
  }
  if (unit_data_point_type_.isValid()) {
    float unit_value = min_unit_ + v_to_unit_slope_ * (voltage - min_v_);
    if (limit_unit_) {
      // Constrain mapped value between min and max unit
      if (min_unit_ < max_unit_) {
        unit_value = std::max(min_unit_, std::min(unit_value, max_unit_));
      } else {
        unit_value = std::max(max_unit_, std::min(unit_value, min_unit_));
      }
    }
    values.push_back({utils::ValueUnit{
        .value = unit_value, .data_point_type = unit_data_point_type_}});
  }

  return {.values = values, .error = ErrorResult()};
}

std::shared_ptr<Peripheral> AnalogIn::factory(
    const ServiceGetters& services, const JsonObjectConst& parameters) {
  return std::make_shared<AnalogIn>(parameters);
}

bool AnalogIn::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

bool AnalogIn::capability_get_values_ =
    capabilities::GetValues::registerType(type());

const std::array<uint8_t, 8> AnalogIn::valid_pins_ = {
    32, 33, 34, 35, 36, 37, 38, 39,
};
const __FlashStringHelper* AnalogIn::pin_key_ = FPSTR("pin");
const __FlashStringHelper* AnalogIn::pin_key_error_ =
    FPSTR("Missing property: pin (unsigned int)");
const __FlashStringHelper* AnalogIn::invalid_pin_error_ =
    FPSTR("Pin # not valid (only ADC1: 32 - 39)");

const __FlashStringHelper* AnalogIn::voltage_data_point_type_key_ =
    FPSTR("voltage_data_point_type");
const __FlashStringHelper* AnalogIn::unit_data_point_type_key_ =
    FPSTR("unit_data_point_type");
const __FlashStringHelper* AnalogIn::percent_data_point_type_key_ =
    FPSTR("percent_data_point_type");
const __FlashStringHelper* AnalogIn::data_point_type_key_error_ =
    FPSTR("No data point type set");
const __FlashStringHelper* AnalogIn::min_v_key_ = FPSTR("min_v");
const __FlashStringHelper* AnalogIn::max_v_key_ = FPSTR("max_v");
const __FlashStringHelper* AnalogIn::min_unit_key_ = FPSTR("min_unit");
const __FlashStringHelper* AnalogIn::max_unit_key_ = FPSTR("max_unit");
const __FlashStringHelper* AnalogIn::limit_unit_key_ = FPSTR("limit_unit");

}  // namespace analog_in
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata