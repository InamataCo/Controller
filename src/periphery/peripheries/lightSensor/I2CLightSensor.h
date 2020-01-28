#ifndef BERND_BOX_PERIPHERY_PERIPHERIES_LIGHTSENSOR_I2CLIGHTSENSOR_H
#define BERND_BOX_PERIPHERY_PERIPHERIES_LIGHTSENSOR_I2CLIGHTSENSOR_H

#include "periphery/peripheries/I2CAbstractPeriphery.h"

namespace bernd_box {
namespace periphery {
namespace peripheries {
namespace light_sensor {

class I2CLightSensor : public I2CAbstractPeriphery {
 public:
  I2CLightSensor(const JsonObjectConst& parameter);
  virtual ~I2CLightSensor() = default;
};

}  // namespace light_sensor
}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box

#endif
