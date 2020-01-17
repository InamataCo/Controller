#ifndef BERND_BOX_TASKS_PUMP_H
#define BERND_BOX_TASKS_PUMP_H

#include "config.h"
#include "managers/io.h"
#include "managers/mqtt.h"
#include "TaskSchedulerDeclarations.h"

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
  const std::chrono::milliseconds getDuration();

 private:
  bool OnEnable() final;
  bool Callback() final;
  void OnDisable() final;
  void MqttCallback(char* topic, uint8_t* payload, unsigned int length);

  Io& io_;
  Mqtt& mqtt_;
  std::chrono::milliseconds start_time_;
};

}  // namespace tasks
}  // namespace bernd_box

#endif
