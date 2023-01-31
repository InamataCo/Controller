#pragma once

#include <ArduinoJson.h>
#include <SparkFunBME280.h>

#include "managers/service_getters.h"
#include "peripheral/capabilities/get_values.h"
#include "peripheral/peripheral.h"
#include "peripheral/peripherals/i2c/i2c_abstract_peripheral.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace bme280 {

class BME280 : public peripherals::i2c::I2CAbstractPeripheral,
               public capabilities::GetValues {
 public:
  BME280(const JsonObjectConst& parameters);
  virtual ~BME280() = default;

  // Type registration in the peripheral factory
  const String& getType() const final;
  static const String& type();

  /**
   * Reads all available data points from the BME/P280
   *
   * \return A vector with all read data points and their type
   */
  capabilities::GetValues::Result getValues() final;

 private:
  static std::shared_ptr<Peripheral> factory(const ServiceGetters& services,
                                             const JsonObjectConst& parameters);
  static bool registered_;
  static bool capability_get_values_;

  utils::UUID temperature_data_point_type_{nullptr};
  static const __FlashStringHelper* temperature_data_point_type_key_;
  static const __FlashStringHelper* temperature_data_point_type_key_error_;

  utils::UUID pressure_data_point_type_{nullptr};
  static const __FlashStringHelper* pressure_data_point_type_key_;
  static const __FlashStringHelper* pressure_data_point_type_key_error_;

  utils::UUID humidity_data_point_type_{nullptr};
  static const __FlashStringHelper* humidity_data_point_type_key_;
  static const __FlashStringHelper* humidity_data_point_type_key_error_;

  /// The supported chip types and their chip IDs used to identify them
  enum class ChipType { BMP280 = 0x58, BME280 = 0x60, Unknown };

  /// The driver to read data from the BME/P280
  ::BME280 driver_;
  /// BME280s I2C address
  uint8_t i2c_address_;
  /// The detected chip type
  ChipType chip_type_ = ChipType::Unknown;

  /// The error if the chip type does not match the expected values
  static const __FlashStringHelper* invalid_chip_type_error_;
};

}  // namespace bme280
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata
