#include "pump.h"

namespace bernd_box {

namespace tasks {

Pump::Pump(Scheduler* scheduler, Io& io, Mqtt& mqtt)
    : Task(scheduler), io_(io), mqtt_(mqtt) {}
Pump::~Pump() {}

void Pump::setDuration(std::chrono::milliseconds duration) {
  setInterval(duration.count());
  setIterations(1);
}

bool Pump::OnEnable() {
  bool isEnableOk = true;

  Result result = io_.setPumpState(true);
  if (result != Result::kSuccess) {
    isEnableOk = false;
    String error = "Failed to setPumpState(true). Result = " + int(result);
    mqtt_.sendError("tasks::Pump::OnEnable", error, true);
  } else {
    Serial.printf("Pumping for %lis\n", getInterval() / 1000);
  }

  return isEnableOk;
}

bool Pump::Callback() { return true; }

void Pump::OnDisable() {
  Serial.println("Finished pumping");

  Result result = io_.setPumpState(false);
  if (result != Result::kSuccess) {
    String error = "Failed to setPumpState(false). Result = " + int(result);
    mqtt_.sendError("tasks::Pump::OnEnable", error, true);
  }
}

}  // namespace tasks

}  // namespace bernd_box