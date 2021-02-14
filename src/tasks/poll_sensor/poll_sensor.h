#pragma once

#include <ArduinoJson.h>

#include <memory>

#include "peripheral/capabilities/start_measurement.h"
#include "tasks/get_values_task/get_values_task.h"

namespace bernd_box {
namespace tasks {
namespace poll_sensor {

/**
 * Polls the sensor via the GetValues capability
 *
 * Uses the StartMeasurement capability if the sensor supports it. The interval
 * parameter is interpreted as the time after a reading has completed.
 * Therefore the total time between readings, if StartMeasurement is supported,
 * is the time to perform the measurement, read the values and then wait the
 * specified time interval.
 *
 * The duration parameter specifies for how long the sensor should be polled.
 * If a measurement has started before the duration ends, it will be completed
 * and sent to the server.
 */
class PollSensor : public get_values_task::GetValuesTask {
 public:
  PollSensor(const JsonObjectConst& parameters, Scheduler& scheduler);
  virtual ~PollSensor() = default;

  const String& getType() const final;
  static const String& type();

  bool TaskCallback() final;

 private:
  static bool registered_;
  static BaseTask* factory(const JsonObjectConst& parameters,
                           Scheduler& scheduler);

  std::chrono::milliseconds interval_;
  std::chrono::steady_clock::time_point run_until_;
  std::shared_ptr<peripheral::capabilities::StartMeasurement>
      start_measurement_peripheral_ = nullptr;
};

}  // namespace poll_sensor
}  // namespace tasks
}  // namespace bernd_box
