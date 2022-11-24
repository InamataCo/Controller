#include "i2c_light_sensor.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace light_sensor {

I2CLightSensor::I2CLightSensor(const JsonObjectConst& parameter)
    : I2CAbstractPeripheral(parameter) {}

}  // namespace light_sensor
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata
