#include "analog_sensors.h"

namespace bernd_box {
namespace tasks {

AnalogSensors::AnalogSensors(Scheduler* scheduler, Io& io, Mqtt& mqtt)
    : Task(scheduler), io_(io), mqtt_(mqtt) {
  Task::setIterations(TASK_FOREVER);
  Task::setInterval(std::chrono::milliseconds(default_period_).count());
}

AnalogSensors::~AnalogSensors() {}

bool AnalogSensors::OnEnable() {
  Serial.println("Starting analog sensor task");

  return true;
}

bool AnalogSensors::Callback() {
  io_.setStatusLed(true);

  // Read all sensors. Then print and send them over MQTT
  Serial.printf("\n%-10s|%-4s|%-15s|%s\n", "Sensor", "Pin", "Value", "Unit");
  Serial.printf("----------|----|---------------|----\n");

  // Measure all
  for (auto& it : io_.adcs_) {
    float value = io_.readAnalog(it.first);

    Serial.printf("%-10s|%-4i|%-15f|%s\n", it.second.name.c_str(),
                  it.second.pin_id, value, it.second.unit.c_str());
    mqtt_.send(it.second.name.c_str(), value);
  }

  io_.setStatusLed(false);

  return true;
}

void AnalogSensors::OnDisable() {
  Task::getStatusRequest();

  Serial.println("Ending analog sensor task");
}

}  // namespace tasks
}  // namespace bernd_box