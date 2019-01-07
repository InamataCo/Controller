#include "light_sensors.h"

namespace bernd_box {
namespace tasks {

LightSensors::LightSensors(Scheduler* scheduler, Io& io, Mqtt& mqtt)
    : Task(scheduler), io_(io), mqtt_(mqtt) {
  setIterations(TASK_FOREVER);
  Task::setInterval(std::chrono::milliseconds(default_period_).count());
}

LightSensors::~LightSensors() {}

bool LightSensors::OnEnable() {
  mqtt_.send("light_sensors_active", "true");
  
  return true;
}

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

void LightSensors::OnDisable() {
  mqtt_.send("light_sensors_active", "false");
}

}  // namespace tasks
}  // namespace bernd_box