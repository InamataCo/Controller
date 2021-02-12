#include "bme280.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace bme280 {

BME280::BME280(const JsonObjectConst& parameters)
    : I2CAbstractPeripheral(parameters) {
  // If the base class constructor failed, abort the constructor
  if (!isValid()) {
    return;
  }

  // Get and check the data point type for temperature readings
  temperature_data_point_type_ =
      utils::UUID(parameters[temperature_data_point_type_key_]);
  if (!temperature_data_point_type_.isValid()) {
    setInvalid(temperature_data_point_type_key_error_);
    return;
  }

  // Get and check the data point type for pressure readings
  pressure_data_point_type_ =
      utils::UUID(parameters[pressure_data_point_type_key_]);
  if (!pressure_data_point_type_.isValid()) {
    setInvalid(pressure_data_point_type_key_error_);
    return;
  }

  // Get and check the I2C address of the BME/P280 chip
  JsonVariantConst i2c_address = parameters[i2c_address_key_];
  if (!i2c_address.is<uint8_t>()) {
    setInvalid(i2c_address_key_error_);
    return;
  }
  i2c_address_ = i2c_address;

  // Do a preliminary check to see if the device is connected to the bus
  if (!isDeviceConnected(i2c_address_)) {
    setInvalid(missingI2CDeviceError(i2c_address_));
    return;
  }

  // Initialize the driver with the correct Wire (I2C) interface
  driver_.setI2CAddress(i2c_address_);
  bool setup_success = driver_.beginI2C(*getWire());
  if (!setup_success) {
    setInvalid(invalid_chip_type_error_);
    return;
  }

  // Get the chip type (BME or BMP). Also checked in beginI2C but not returned
  uint8_t chip_id = driver_.readRegister(BME280_CHIP_ID_REG);
  if (chip_id == static_cast<uint8_t>(ChipType::BMP280)) {
    chip_type_ = ChipType::BMP280;
  } else if (chip_id == static_cast<uint8_t>(ChipType::BME280)) {
    chip_type_ = ChipType::BME280;

    // Only the BME280 has humidity readings
    humidity_data_point_type_ =
        utils::UUID(parameters[humidity_data_point_type_key_]);
    if (!humidity_data_point_type_.isValid()) {
      setInvalid(humidity_data_point_type_key_error_);
      return;
    }
  } else {
    setInvalid(invalid_chip_type_error_);
    return;
  }
}

const String& BME280::getType() const { return type(); }

const String& BME280::type() {
  static const String name{"BME280"};
  return name;
}

capabilities::GetValues::Result BME280::getValues() {
  if (!isDeviceConnected(i2c_address_)) {
    return {.values = {},
            .error = ErrorResult(type(), missingI2CDeviceError(i2c_address_))};
  }

  capabilities::GetValues::Result result;

  result.values.push_back(
      utils::ValueUnit{.value = driver_.readTempC(),
                       .data_point_type = temperature_data_point_type_});
  result.values.push_back(
      utils::ValueUnit{.value = driver_.readFloatPressure(),
                       .data_point_type = pressure_data_point_type_});
  if (chip_type_ == ChipType::BME280) {
    result.values.push_back(
        utils::ValueUnit{.value = driver_.readFloatHumidity(),
                         .data_point_type = humidity_data_point_type_});
  }
  return result;
}

std::shared_ptr<Peripheral> BME280::factory(const JsonObjectConst& parameters) {
  return std::make_shared<BME280>(parameters);
}

bool BME280::registered_ = PeripheralFactory::registerFactory(type(), factory);

bool BME280::capability_get_values_ =
    capabilities::GetValues::registerType(type());

const __FlashStringHelper* BME280::temperature_data_point_type_key_ =
    F("temperature_data_point_type");
const __FlashStringHelper* BME280::temperature_data_point_type_key_error_ =
    F("Missing property: temperature_data_point_type (UUID)");

const __FlashStringHelper* BME280::pressure_data_point_type_key_ =
    F("pressure_data_point_type");
const __FlashStringHelper* BME280::pressure_data_point_type_key_error_ =
    F("Missing property: pressure_data_point_type (UUID)");

const __FlashStringHelper* BME280::humidity_data_point_type_key_ =
    F("humidity_data_point_type");
const __FlashStringHelper* BME280::humidity_data_point_type_key_error_ =
    F("Missing property: humidity_data_point_type (UUID)");

const __FlashStringHelper* BME280::invalid_chip_type_error_ =
    F("Failed BME/P280 setup");

}  // namespace bme280
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box