#include "pump.h"

namespace bernd_box {

namespace tasks {

Pump::Pump(Scheduler* scheduler, Io& io, Mqtt& mqtt)
    : Task(scheduler), io_(io), mqtt_(mqtt) {
  setIterations(TASK_FOREVER);
  setInterval(TASK_HOUR);  // All logic is in OnEnable and OnDisable
}
Pump::~Pump() {}

void Pump::setDuration(std::chrono::milliseconds duration) {
  setTimeout(std::chrono::milliseconds(duration).count());
}

bool Pump::OnEnable() {
  bool isEnableOk = true;

  Result result = io_.setPumpState(true);
  if (result != Result::kSuccess) {
    isEnableOk = false;
    String error = "Failed to setPumpState(true). Result = " + int(result);
    mqtt_.sendError("tasks::Pump::OnEnable", error, true);
  } else {
    Serial.printf("Pumping for %lus\n", getTimeout() / 1000);
  }

  if (isEnableOk) {
    mqtt_.send("pump_active", "true");
  }

  return isEnableOk;
}

bool Pump::Callback() { return true; }

void Pump::OnDisable() {
  Serial.println("Finished pumping");

  setTimeout(TASK_NOTIMEOUT);
  Result result = io_.setPumpState(false);
  if (result != Result::kSuccess) {
    String error = "Failed to setPumpState(false). Result = " + int(result);
    mqtt_.sendError("tasks::Pump::OnDisable", error, true);
  } else {
    mqtt_.send("pump_active", "false");
  }
}

}  // namespace tasks

}  // namespace bernd_box