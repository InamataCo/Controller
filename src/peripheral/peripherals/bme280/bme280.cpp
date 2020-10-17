#include "bme280.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace bme280 {

BME280::BME280(const JsonObjectConst& parameters)
    : I2CAbstractPeripheral(parameters) {
  JsonVariantConst i2c_address = parameters[i2c_address_key_];
  if (!i2c_address.is<uint16_t>()) {
    setInvalid(i2c_address_key_error_);
    return;
  }

  if (!isDeviceConnected(i2c_address)) {
    setInvalid(missingI2CDeviceError(i2c_address.as<uint16_t>()));
    return;
  }

  driver_.setI2CAddress(i2c_address);
  bool setup_success = driver_.beginI2C(*getWire());
  if (!setup_success) {
    setInvalid(failed_setup_error_);
    return;
  }
}

const String& BME280::getType() const { return type(); }

const String& BME280::type() {
  static const String name{"BME280"};
  return name;
}

std::vector<capabilities::ValueUnit> BME280::getValues() {
  std::vector<capabilities::ValueUnit> values;
  values.push_back(
      capabilities::ValueUnit{.value = driver_.readTempC(), .unit = "°C"});
  values.push_back(capabilities::ValueUnit{.value = driver_.readFloatPressure(),
                                           .unit = "Pa"});
  if (chip_type_ == ChipType::BME280) {
    values.push_back(capabilities::ValueUnit{
        .value = driver_.readFloatHumidity(), .unit = "%RH"});
  }
  return values;
}

std::shared_ptr<Peripheral> BME280::factory(const JsonObjectConst& parameters) {
  return std::make_shared<BME280>(parameters);
}

bool BME280::registered_ = PeripheralFactory::registerFactory(type(), factory);

bool BME280::capability_get_values_ =
    capabilities::GetValues::registerType(type());

const __FlashStringHelper* BME280::failed_setup_error_ =
    F("Failed BME/P280 setup");

}  // namespace bme280
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box