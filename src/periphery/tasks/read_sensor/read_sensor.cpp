#include "read_sensor.h"

namespace bernd_box {
namespace periphery {
namespace tasks {

ReadSensor::ReadSensor(std::shared_ptr<Periphery> periphery,
                       const JsonObjectConst& parameters)
    : PeripheryTask(periphery) {}

const __FlashStringHelper* ReadSensor::getType() { return type(); }

const __FlashStringHelper* ReadSensor::type() { return F("ReadSensor"); }

bool ReadSensor::OnEnable() { return true; }

bool ReadSensor::Callback() { return false; }

bool ReadSensor::registered_ =
    PeripheryTaskFactory::registerTask(type(), factory);

std::unique_ptr<PeripheryTask> ReadSensor::factory(
    std::shared_ptr<Periphery> periphery, const JsonObjectConst& parameters) {
  return std::unique_ptr<ReadSensor>(new ReadSensor(periphery, parameters));
}

}  // namespace tasks
}  // namespace periphery
}  // namespace bernd_box
