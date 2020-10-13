#include "bme280.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace bme280 {

BME280::BME280(const JsonObjectConst& parameters)
    : I2CAbstractPeripheral(parameters) {
  JsonVariantConst i2c_address = parameters[i2c_address_key_];
  if (!i2c_address.is<uint16_t>()) {
    Services::getServer().sendError(
        type(),
        String(F("Missing property: ")) + i2c_address_key_ + F(" (uint16_t)"));
    setInvalid();
    return;
  }

  if (!isDeviceConnected(i2c_address)) {
    Services::getServer().sendError(
        type(), String(F("Cannot find device with I2C address: ")) +
                    i2c_address.as<int>());
    setInvalid();
    return;
  }

  driver_.setI2CAddress(i2c_address);
  uint8_t chip_id = driver_.beginI2C();

  if (chip_id == 0x58) {
    chip_type_ = ChipType::BMP280;
  } else if (chip_id == 0x60) {
    chip_type_ = ChipType::BME280;
  } else {
    Services::getServer().sendError(
        type(), String(F("Invalid BME/P280 chip ID: ")) + chip_id);
    setInvalid();
    return;
  }
}

const String& BME280::getType() { return type(); }

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

const __FlashStringHelper* BME280::i2c_address_key_ = F("i2c_address");

std::shared_ptr<Peripheral> BME280::factory(const JsonObjectConst& parameters) {
  return std::make_shared<BME280>(parameters);
}

bool BME280::registered_ = PeripheralFactory::registerFactory(type(), factory);

bool BME280::capability_get_values_ =
    capabilities::GetValues::registerType(type());

}  // namespace bme280
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box