#ifndef BERND_BOX_TASKS_PUMP_H
#define BERND_BOX_TASKS_PUMP_H

#include "task.h"

#include "io.h"
#include "mqtt.h"

namespace bernd_box {
namespace tasks {

/**
 * Starts pumping when enabled and stops after timeout.
 *
 * Set the pump duration in seconds with setDuration() and then call enable()
 */
class Pump : public Task {
 public:
  Pump(Scheduler* scheduler, Io& io, Mqtt& mqtt);
  virtual ~Pump();

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