#ifndef BERND_BOX_TASKS_DISSOLVED_OXYGEN_SENSOR_H
#define BERND_BOX_TASKS_DISSOLVED_OXYGEN_SENSOR_H

#include "TaskSchedulerDeclarations.h"

#include "managers/io.h"
#include "managers/mqtt.h"

namespace bernd_box {
namespace tasks {

class DissolvedOxygenSensor : public Task {
 private:
  const std::chrono::seconds default_period_{1};

  /// Number of samples to take (size of buffer)
  static const uint sample_count_ = 60;

  /// The voltage and temperature for the saturated dissolved oxygen sensor
  float voltage_v_at_saturated_do_ = 1.330151;
  float temperature_c_at_saturated_do = 22.8125;

 public:
  DissolvedOxygenSensor(Scheduler* scheduler, Io& io, Mqtt& mqtt);
  virtual ~DissolvedOxygenSensor();

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

  float getLastMeasurement();

  /**
   * Returns true if the measurement buffer is full
   */
  bool isMeasurementFull();

  void setTemperature(float temperature_c);

  void setCalibrationMode(bool is_calibration_mode_);

 private:
  /**
   * Takes an acidity measurement and saves it in the ring buffer
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

  /// Buffer for the dissolved oxygen samples
  std::array<float, sample_count_> samples_;
  uint sample_index_ = 0;

  /// Current water temperature
  float temperature_c_;

  bool is_calibration_mode_;

  /// Sensor ID to be measured (used in readAnalogV())
  int used_sensor_;

  const std::array<float, 41> temperatureCoefficient = {
      {14.46, 14.22, 13.82, 13.44, 13.09, 12.74, 12.42, 12.11, 11.81,
       11.53, 11.26, 11.01, 10.77, 10.53, 10.3,  10.08, 9.86,  9.66,
       9.46,  9.27,  9.08,  8.9,   8.73,  8.57,  8.41,  8.25,  8.11,
       7.96,  7.82,  7.69,  7.56,  7.43,  7.3,   7.18,  7.07,  6.95,
       6.84,  6.73,  6.63,  6.53,  6.41}};
};

}  // namespace tasks
}  // namespace bernd_box

#endif
