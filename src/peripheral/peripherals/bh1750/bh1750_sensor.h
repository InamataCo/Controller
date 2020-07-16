#pragma once

#include "peripheral/peripherals/light_sensor/i2c_light_sensor.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace bh1750 {

class Bh1750Sensor : public light_sensor::I2CLightSensor {
  Bh1750Sensor(const JsonObjectConst& parameter);
  virtual ~Bh1750Sensor() = default;
};

}
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
