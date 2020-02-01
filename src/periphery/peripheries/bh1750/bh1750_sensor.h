#ifndef BERND_BOX_PERIPHERY_PERIPHERIES_BH1750_BH1750_SENSOR_H
#define BERND_BOX_PERIPHERY_PERIPHERIES_BH1750_BH1750_SENSOR_H

#include "periphery/peripheries/lightSensor/i2c_light_sensor.h"

namespace bernd_box {
namespace periphery {
namespace peripheries {
namespace bh1750 {

class Bh1750Sensor : public light_sensor::I2CLightSensor {
  Bh1750Sensor(const JsonObjectConst& parameter);
  virtual ~Bh1750Sensor() = default;
};

}
}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box

#endif
