#ifndef BERND_BOX_TASK_ACIDITY_SENSOR_H
#define BERND_BOX_TASK_ACIDITY_SENSOR_H

#include "task.h"

#include "io.h"
#include "mqtt.h"

namespace bernd_box {
namespace tasks {

/**
 * Take multiple acidity readings and average them. Task stops after enough
 * measurements have been collected
 */
class AciditySensor : public Task {
 private:
  const std::chrono::milliseconds default_period_{500};

  /// Number of samples to take (size of buffer)
  static const uint sample_count_ = 30;

 public:
  /// Conversion from voltage to pH -> pH = V * factor - offset
  const float acidity_factor_v_to_ph = 3.5;

  /// pH too high above reference. Will be substracted from each measurement
  float acidity_offset = 0.4231628418;

  AciditySensor(Scheduler* scheduler, Io& io, Mqtt& mqtt,
                Sensor used_sensor = Sensor::kAciditiy);
  virtual ~AciditySensor();

  /**
   * Gets the median value of the measurements
   *
   * Sorts the values in place, which will cause the order to change. Assumes
   * all valid measurements are before the first NAN.
   */
  float getMedianMeasurement();

  /**
   * Returns true if the measurement buffer is full
   */
  bool isMeasurementFull();

 private:
  /**
   * Takes a dissolved oxygen measurement and saves it in the ring buffer
   */
  void takeMeasurement();

  /**
   * Clears all acidity measurements
   */
  void clearMeasurements();

  bool OnEnable() final;
  bool Callback() final;
  void OnDisable() final;

  Io& io_;
  Mqtt& mqtt_;

  /// Buffer for the acidity samples
  std::array<float, sample_count_> samples_;
  uint sample_index_ = 0;

  /// Sensor ID to be measured (used in readAnalogV())
  const Sensor used_sensor_;
};

}  // namespace tasks
}  // namespace bernd_box

#endif