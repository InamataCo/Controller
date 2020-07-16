#pragma once

#include <Wire.h>
#include "peripheral/peripheral.h"
#include "peripheral/peripherals/util/i2c_adapter.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {

/**
 * Interface for sensors to access peripherals over the I2C bus
 */
class I2CAbstractPeripheral : public Peripheral {
 public:
  I2CAbstractPeripheral(const JsonObjectConst& parameter);
  virtual ~I2CAbstractPeripheral() = default;

 protected:
  TwoWire* getWire();

 private:
  std::shared_ptr<peripherals::util::I2CAdapter> i2c_adapter_;
};

}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
