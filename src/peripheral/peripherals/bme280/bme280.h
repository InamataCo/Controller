#pragma once

#include <ArduinoJson.h>
#include <SparkFunBME280.h>

#include "peripheral/capabilities/get_values.h"
#include "peripheral/peripheral.h"
#include "peripheral/peripherals/i2c_adapter/i2c_abstract_peripheral.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace bme280 {

class BME280 : public peripherals::i2c_adapter::I2CAbstractPeripheral,
               public capabilities::GetValues {
 public:
  BME280(const JsonObjectConst& parameters);
  virtual ~BME280() = default;

  // Type registration in the peripheral factory
  const String& getType() final;
  static const String& type();

  std::vector<capabilities::ValueUnit> getValues() final;

  /// Name of the parameter that holds the I2C address
  static const __FlashStringHelper* i2c_address_key_;

 private:
  static std::shared_ptr<Peripheral> factory(const JsonObjectConst& parameters);
  static bool registered_;
  static bool capability_get_values_;

  enum class ChipType {
    BMP280,
    BME280,
    Unknown
  };

  ::BME280 driver_;
  ChipType chip_type_ = ChipType::Unknown;
};

}  // namespace bme280
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
