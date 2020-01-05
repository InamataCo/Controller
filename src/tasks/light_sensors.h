#ifndef BERND_BOX_TASKS_LIGHT_SENSORS_H
#define BERND_BOX_TASKS_LIGHT_SENSORS_H

#include "task.h"

#include "managers/io.h"
#include "managers/mqtt.h"

namespace bernd_box {
namespace tasks {

/**
 * Reads, prints and then sends all light sensors
 */
class LightSensors : public Task {
 private:
  const std::chrono::seconds default_period_{1};

 public:
  LightSensors(Scheduler* scheduler, Io& io, Mqtt& mqtt);
  virtual ~LightSensors();

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
