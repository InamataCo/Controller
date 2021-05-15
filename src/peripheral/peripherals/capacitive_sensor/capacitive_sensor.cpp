#include "capacitive_sensor.h"

#include "peripheral/peripheral_factory.h"
#include "utils/value_unit.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace capacative_sensor {

CapacitiveSensor::CapacitiveSensor(const JsonObjectConst& parameters) {
  JsonVariantConst sense_pin = parameters[sense_pin_key_];
  if (!sense_pin.is<unsigned int>()) {
    setInvalid(sense_pin_key_error_);
    return;
  }

  sense_pin_ = sense_pin;

  data_point_type_ = parameters[utils::ValueUnit::data_point_type_key];
  if (!data_point_type_.isValid()) {
    setInvalid(utils::ValueUnit::data_point_type_key_error);
    return;
  }
}

const String& CapacitiveSensor::getType() const { return type(); }

const String& CapacitiveSensor::type() {
  static const String name{"CapacitiveSensor"};
  return name;
}

capabilities::GetValues::Result CapacitiveSensor::getValues() {
  return {.values = {utils::ValueUnit{
              .value = static_cast<float>(touchRead(sense_pin_)),
              .data_point_type = data_point_type_}}};
}

const __FlashStringHelper* CapacitiveSensor::sense_pin_key_ = F("sense_pin");
const __FlashStringHelper* CapacitiveSensor::sense_pin_key_error_ =
    F("Missing property: sense_pin (unsigned int)");

std::shared_ptr<Peripheral> CapacitiveSensor::factory(
    const ServiceGetters& services, const JsonObjectConst& parameters) {
  return std::make_shared<CapacitiveSensor>(parameters);
}

bool CapacitiveSensor::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

bool CapacitiveSensor::capability_get_value_ =
    capabilities::GetValues::registerType(type());

}  // namespace capacative_sensor
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
