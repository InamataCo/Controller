#pragma once

#include "peripheral/peripherals/i2c/i2c_abstract_peripheral.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace light_sensor {

class I2CLightSensor : public peripherals::i2c::I2CAbstractPeripheral {
 public:
  I2CLightSensor(const JsonObjectConst& parameter);
  virtual ~I2CLightSensor() = default;
};

}  // namespace light_sensor
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata
