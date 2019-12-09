#ifndef BERND_BOX_TASKS_DALLAS_TEMPERATURE_H
#define BERND_BOX_TASKS_DALLAS_TEMPERATURE_H

#include "sensor_types.h"
#include "tasks/task.h"

#include "io.h"
#include "mqtt.h"

#include <array>

namespace bernd_box {
namespace tasks {

class DallasTemperature : public Task {
 public:
  const std::chrono::seconds sleep_between_measurements_{1};
  
  bool has_new_samples_ = false; 

  static const uint sample_count_ = 5;

  DallasTemperature(Scheduler* scheduler, Io& io, Mqtt& mqtt);
  virtual ~DallasTemperature();

  /**
   * Gets last stored temperature from the buffer
   *
   * \return Temperature in °C
   */
  Measurement getLastSample();

 private:
  bool OnEnable() final;
  bool Callback() final;
  void OnDisable() final;

  /**
   * Pushes the temperature measurement into the ring buffer
   *
   * \param temperature The temperature in °C
   * \param sensorId ID of the sensor which was measured
   */
  void pushSample(const float temperature_c, Sensor sensorId);

  /**
   * Clear the ring buffer and reset the index
   */
  void clearSamples();

  Io& io_;
  Mqtt& mqtt_;

  // Buffer for the last temperature measurement <°C, millis timestamp>
  std::array<Measurement, sample_count_> samples_{};
  uint sample_index_ = 0;

  bool is_request_sent = false;
};

}  // namespace tasks
}  // namespace bernd_box

#endif