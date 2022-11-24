#pragma once

#include <Wire.h>

#include "peripheral/peripheral.h"
#include "peripheral/peripherals/i2c_adapter/i2c_adapter.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace i2c_adapter {

/**
 * Interface for sensors to access peripherals over the I2C bus
 */
class I2CAbstractPeripheral : public Peripheral {
 public:
  I2CAbstractPeripheral(const JsonObjectConst& parameter);
  virtual ~I2CAbstractPeripheral() = default;

  static const String& type();

 protected:
  TwoWire* getWire();
  bool isDeviceConnected(uint16_t i2c_address);

  static String missingI2CDeviceError(int i2c_address);

  static const __FlashStringHelper* i2c_address_key_;
  static const __FlashStringHelper* i2c_address_key_error_;

 private:
  static String invalidI2CAdapterError(const utils::UUID& uuid,
                                       const String& type);

  static const __FlashStringHelper* i2c_adapter_key_;
  static const __FlashStringHelper* i2c_adapter_key_error_;

  std::shared_ptr<peripherals::util::I2CAdapter> i2c_adapter_;
};

}  // namespace i2c_adapter
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata
