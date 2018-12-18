#include "light_sensors.h"

namespace bernd_box {
namespace tasks {

LightSensors::LightSensors(Scheduler* scheduler, Io& io, Mqtt& mqtt)
    : Task(scheduler), io_(io), mqtt_(mqtt) {}

LightSensors::~LightSensors() {}

bool LightSensors::Callback() {
  io_.setStatusLed(true);

  for (const auto& max44009 : io_.max44009s_) {
    float value = io_.readMax44009Light(max44009.first);
    Serial.printf("Ambient brightness (ID: %u) is %f %s\n",
                  static_cast<int>(max44009.first), value,
                  max44009.second.unit.c_str());
    mqtt_.send(max44009.second.name.c_str(), value);
  }

  io_.setStatusLed(false);

  return true;
}

}  // namespace tasks
}  // namespace bernd_box