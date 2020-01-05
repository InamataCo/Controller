#include "dissolved_oxygen_sensor.h"

namespace bernd_box {
namespace tasks {
DissolvedOxygenSensor::DissolvedOxygenSensor(Scheduler* scheduler, Io& io,
                                             Mqtt& mqtt)
    : Task(scheduler), io_(io), mqtt_(mqtt), is_calibration_mode_(false) {
  setIterations(TASK_FOREVER);
  Task::setInterval(std::chrono::milliseconds(default_period_).count());
  clearMeasurements();
}

DissolvedOxygenSensor::~DissolvedOxygenSensor() {}

void DissolvedOxygenSensor::setSensorId(const int sensor_id) {
  clearMeasurements();
  used_sensor_ = sensor_id;
}

bool DissolvedOxygenSensor::isReady() { return used_sensor_ > -1; }

float DissolvedOxygenSensor::getMedianMeasurement() {
  // Find how many measurements have been stored. Either the first element
  // before a NaN element, or the complete vector
  size_t sample_count = 0;
  while (sample_count < samples_.size() &&
         !std::isnan(samples_[sample_count])) {
    sample_count++;
  }

  std::sort(samples_.begin(), samples_.begin() + sample_count);

  float median;
  if (sample_count % 2 == 0) {
    median = (samples_[sample_count / 2 - 1] + samples_[sample_count / 2]) / 2;
  } else {
    median = samples_[sample_count / 2];
  }

  for (const auto& it : samples_) {
    Serial.printf("%f, ", it);
  }
  Serial.printf("\n");
  Serial.printf("Median: %f\n", median);

  // Returns the middle element
  return median;
}

float DissolvedOxygenSensor::getLastMeasurement() {
  return samples_[sample_index_];
}

bool DissolvedOxygenSensor::isMeasurementFull() {
  return !std::isnan(samples_.back());
}

void DissolvedOxygenSensor::setTemperature(float temperature_c) {
  temperature_c_ = temperature_c;
}

void DissolvedOxygenSensor::takeMeasurement() {
  const float almost_zero = 0.001;
  float analog_v = io_.readAnalogV(used_sensor_);

  if (std::isnan(analog_v)) {
    Serial.printf("Error updating acidity sensor. Returned NAN\n");
    return;
  }

  // Check that the value is non-zero
  if (fabs(analog_v) < almost_zero) {
    Serial.printf("Discarding acidity measurement (%fV) as almost zero (%f)\n",
                  analog_v, almost_zero);
    return;
  }

  float sample_value;
  if (is_calibration_mode_) {
    float v_at_saturated_do_ = analog_v;
    temperature_c_at_saturated_do = temperature_c_;
    sample_value = v_at_saturated_do_;
  } else {
    float do_percent =
        analog_v * temperatureCoefficient[temperature_c_at_saturated_do + 0.5] /
        voltage_v_at_saturated_do_;
    sample_value = do_percent;
    Serial.printf("DO = %fmg/L, Spannung = %fV\n", do_percent, analog_v);
  }

  // Reuse current slot if it is unused
  if (!std::isnan(samples_[sample_index_])) {
    sample_index_++;

    if (sample_index_ >= samples_.size()) {
      sample_index_ = 0;
    }
  }

  samples_[sample_index_] = sample_value;
}

void DissolvedOxygenSensor::clearMeasurements() {
  samples_.fill(NAN);
  sample_index_ = 0;
}

bool DissolvedOxygenSensor::OnEnable() {
  Result result = io_.enableAnalog(used_sensor_);
  if (result != Result::kSuccess) {
    String error =
        "Error on enabling analog sensor. Result = " + String(int(result));
    mqtt_.sendError("DissolvedOxygenSensor::OnEnable", error, true);
  }

  clearMeasurements();

  if (result == Result::kSuccess) {
    mqtt_.send("dissolved_oxygen_sensor_active", "true");
  }

  return result == Result::kSuccess;
}

bool DissolvedOxygenSensor::Callback() {
  io_.setStatusLed(true);

  takeMeasurement();

  // Once enough measurements have been taken, stop the task
  if (isMeasurementFull()) {
    disable();
  }

  io_.setStatusLed(false);
  return true;
}

void DissolvedOxygenSensor::OnDisable() {
  io_.disableAnalog(used_sensor_);
  mqtt_.send("dissolved_oxygen_sensor_active", "false");
}

}  // namespace tasks
}  // namespace bernd_box
