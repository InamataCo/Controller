#include "bh1750_sensor.h"

namespace bernd_box {
namespace peripheral {
namespace peripheries {
namespace bh1750 {

Bh1750Sensor::Bh1750Sensor(const JsonObjectConst& parameter)
    : light_sensor::I2CLightSensor(parameter) {}

}  // namespace bh1750
}  // namespace peripheries
}  // namespace peripheral
}  // namespace bernd_box
