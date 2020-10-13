#pragma once

#include "peripheral/peripherals/i2c_adapter/i2c_abstract_peripheral.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace light_sensor {

class I2CLightSensor : public peripherals::i2c_adapter::I2CAbstractPeripheral {
 public:
  I2CLightSensor(const JsonObjectConst& parameter);
  virtual ~I2CLightSensor() = default;
};

}  // namespace light_sensor
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
