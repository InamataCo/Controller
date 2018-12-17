#ifndef BERND_BOX_TASKS_SENSORS_H
#define BERND_BOX_TASKS_SENSORS_H

#define _TASK_STATUS_REQUEST
#define _TASK_OO_CALLBACKS

#include <TaskSchedulerDeclarations.h>

#include "io.h"
#include "mqtt.h"

namespace bernd_box {

namespace tasks {

/**
 * Starts pumping when enabled and stops after timeout.
 *
 * Set the pump duration in seconds with setDuration() and then call start()
 */
class Pump : public Task {
 public:
  Pump(Scheduler* scheduler, Io& io, Mqtt& mqtt);
  virtual ~Pump();

  void setDuration(std::chrono::milliseconds duration);
  void start();

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