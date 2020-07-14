#ifndef BERND_BOX_PERIPHERY_PERIPHERIES_LIGHTSENSOR_I2CLIGHTSENSOR_H
#define BERND_BOX_PERIPHERY_PERIPHERIES_LIGHTSENSOR_I2CLIGHTSENSOR_H

#include "peripheral/peripherals/i2c_abstract_peripheral.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace light_sensor {

class I2CLightSensor : public peripherals::I2CAbstractPeripheral {
 public:
  I2CLightSensor(const JsonObjectConst& parameter);
  virtual ~I2CLightSensor() = default;
};

}  // namespace light_sensor
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box

#endif
