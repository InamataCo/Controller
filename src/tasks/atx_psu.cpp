#include "atx_psu.h"

namespace bernd_box {

namespace tasks {

AtxPsu::AtxPsu(Scheduler* scheduler, Io& io, Mqtt& mqtt)
    : Task(scheduler), io_(io), mqtt_(mqtt) {
  setIterations(TASK_FOREVER);
  setInterval(TASK_HOUR);  // All logic is in OnEnable and OnDisable. Use max interval
}
AtxPsu::~AtxPsu() {}

bool AtxPsu::OnEnable() {
  io_.setPinState(io_.atx_power_pin_, true);
  mqtt_.send("atx_psu_active", "true");
  return true;
}

bool AtxPsu::Callback() { return true; }

void AtxPsu::OnDisable() {
  io_.setPinState(io_.atx_power_pin_, true);
  mqtt_.send("atx_psu_active", "false");
}

}  // namespace tasks

}  // namespace bernd_box
