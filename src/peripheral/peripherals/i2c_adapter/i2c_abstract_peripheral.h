#pragma once

#include <Wire.h>

#include "managers/services.h"
#include "peripheral/peripheral.h"
#include "peripheral/peripherals/i2c_adapter/i2c_adapter.h"

namespace bernd_box {
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

 protected:
  TwoWire* getWire();
  bool isDeviceConnected(uint16_t i2c_address);

 private:
  std::shared_ptr<peripherals::util::I2CAdapter> i2c_adapter_;
};

}  // namespace i2c_adapter
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
