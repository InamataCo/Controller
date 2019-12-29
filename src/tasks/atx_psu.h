#ifndef BERND_BOX_TASKS_PUMP_H
#define BERND_BOX_TASKS_PUMP_H

#include "task.h"

#include "io.h"
#include "mqtt.h"

namespace bernd_box {
namespace tasks {

/**
 * Powers on the ATX power supply.
 */
class AtxPsu : public Task {
 public:
  AtxPsu(Scheduler* scheduler, Io& io, Mqtt& mqtt);
  virtual ~AtxPsu();

  void setDuration(std::chrono::milliseconds duration);

 private:
  bool OnEnable() final;
  bool Callback() final;
  void OnDisable() final;

  Io& io_;
  Mqtt& mqtt_;
};

}  // namespace tasks
}  // namespace bernd_box

#endif
