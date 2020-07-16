#pragma once

#include "TaskSchedulerDeclarations.h"

#include "managers/io.h"
#include "managers/mqtt.h"

namespace bernd_box {
namespace tasks {

/**
 * Take multiple acidity readings and average them. Task stops after enough
 * measurements have been collected
 */
class AciditySensor : public Task {
 private:
  /// The period with which the task should be activated by default
  const std::chrono::milliseconds default_period_{500};

  /// Number of samples to take (size of buffer)
  static const uint sample_count_ = 30;

 public:
  /// Conversion from voltage to pH -> pH = V * factor - offset
  const float acidity_factor_v_to_ph = 3.5;

  /// pH too high above reference. Will be substracted from each measurement
  float acidity_offset_ph = -0.42;

  AciditySensor(Scheduler* scheduler, Io& io, Mqtt& mqtt);
  virtual ~AciditySensor();

  /**
   * Sets the used sensor ID and clears measurements
   * 
   * \param sensor_id The value of the new sensor ID
   */
  void setSensorId(const int sensor_id);

  /**
   * Checks if the task is ready to be enabled.
   */
  bool isReady();

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

  /**
   * Checks whether the analog 
   */
  bool OnEnable() final;
  bool Callback() final;
  void OnDisable() final;

  Io& io_;
  Mqtt& mqtt_;

  /// Buffer for the acidity samples
  std::array<float, sample_count_> samples_;
  uint sample_index_ = 0;

  /// Sensor ID to be measured (used in readAnalogV())
  int used_sensor_;
};

}  // namespace tasks
}  // namespace bernd_box
