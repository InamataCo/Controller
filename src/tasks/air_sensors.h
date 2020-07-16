#pragma once

#include "TaskSchedulerDeclarations.h"

#include "managers/io.h"
#include "managers/mqtt.h"

namespace bernd_box {
namespace tasks {

/**
 * Reads, prints and then sends all air sensor parameters
 */
class AirSensors : public Task {
 private:
  const std::chrono::seconds default_period_{1};

 public:
  AirSensors(Scheduler* scheduler, Io& io, Mqtt& mqtt);
  virtual ~AirSensors();

 private:
  bool OnEnable() final;
  bool Callback() final;
  void OnDisable() final;

  Io& io_;
  Mqtt& mqtt_;
};

}  // namespace tasks
}  // namespace bernd_box
