#ifndef BERND_BOX_TASKS_LIGHT_SENSORS_H
#define BERND_BOX_TASKS_LIGHT_SENSORS_H

#include "task.h"

#include "io.h"
#include "mqtt.h"

namespace bernd_box {
namespace tasks {

/**
 * Reads, prints and then sends all light sensors
 */
class LightSensors : public Task {
 public:
  LightSensors(Scheduler* scheduler, Io& io, Mqtt& mqtt);
  virtual ~LightSensors();

 private:
  bool Callback();

  Io& io_;
  Mqtt& mqtt_;
};

}  // namespace tasks
}  // namespace bernd_box

#endif