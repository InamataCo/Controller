#pragma once

#include "TaskSchedulerDeclarations.h"

#include "managers/io.h"
#include "managers/mqtt.h"

namespace bernd_box {
namespace tasks {

/**
 * Read and then print the analog sensors
 */
class AnalogSensors : public Task {
 private:
  const std::chrono::seconds default_period_{1};

 public:
  AnalogSensors(Scheduler* scheduler, Io& io, Mqtt& mqtt);
  virtual ~AnalogSensors();

 private:
  bool OnEnable() final;
  bool Callback() final;
  void OnDisable() final;

  Io& io_;
  Mqtt& mqtt_;
};

}  // namespace tasks
}  // namespace bernd_box
