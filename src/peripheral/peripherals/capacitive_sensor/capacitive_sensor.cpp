#include "capacitive_sensor.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace capacative_sensor {

CapacitiveSensor::CapacitiveSensor(const JsonObjectConst& parameter) {
  JsonVariantConst sense_pin = parameter[sense_pin_name_];
  if (!sense_pin.is<unsigned int>()) {
    Services::getServer().sendError(type(), String(F("Missing property: ")) +
                                                sense_pin_name_ +
                                                F(" (unsigned int)"));
    setInvalid();
    return;
  }

  sense_pin_ = sense_pin;
}

const String& CapacitiveSensor::getType() { return type(); }

const String& CapacitiveSensor::type() {
  static const String name{"CapacitiveSensor"};
  return name;
}

float CapacitiveSensor::getValue() { return touchRead(sense_pin_); }

const __FlashStringHelper* CapacitiveSensor::sense_pin_name_ = F("sense_pin");

std::shared_ptr<Peripheral> CapacitiveSensor::factory(
    const JsonObjectConst& parameter) {
  return std::make_shared<CapacitiveSensor>(parameter);
}

bool CapacitiveSensor::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

bool CapacitiveSensor::capability_get_value_ =
    capabilities::GetValue::registerType(type());

}  // namespace capacative_sensor
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
