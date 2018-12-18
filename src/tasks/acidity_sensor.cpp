#include "acidity_sensor.h"

namespace bernd_box {
namespace tasks {

AciditySensor::AciditySensor(Scheduler* scheduler, Io& io, Mqtt& mqtt,
                             Sensor used_sensor)
    : Task(scheduler), io_(io), mqtt_(mqtt), used_sensor_(used_sensor) {
  samples_.fill(NAN);
}

AciditySensor::~AciditySensor() {}

float AciditySensor::getMedianMeasurement() {
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

bool AciditySensor::isMeasurementFull() { return !std::isnan(samples_.back()); }

void AciditySensor::takeMeasurement() {
  const float almost_zero = 0.001;
  float analog_v = io_.readAnalogV(used_sensor_);

  if (std::isnan(analog_v)) {
    Serial.printf("Error updating acidity sensor. Returned NAN\n");
    return;
  }

  // Check that the value is non-zero
  if (abs(analog_v) < almost_zero) {
    Serial.printf("Discarding acidity measurement (%fV) as almost zero (%f)\n",
                  analog_v, almost_zero);
    return;
  }

  float acidity_ph = analog_v * acidity_factor_v_to_ph - acidity_offset;

  // Reuse current slot if it is unused
  if (!std::isnan(samples_[sample_index_])) {
    sample_index_++;

    if (sample_index_ >= samples_.size()) {
      sample_index_ = 0;
    }
  }

  samples_[sample_index_] = acidity_ph;
}

void AciditySensor::clearMeasurements() {
  for (auto& it : samples_) {
    it = NAN;
  }

  sample_index_ = 0;
}

bool AciditySensor::OnEnable() {
  Result result = io_.enableAnalog(bernd_box::Sensor::kAciditiy);
  if (result != Result::kSuccess) {
    String error =
        "Error on enabling analog sensor. Result = " + String(int(result));
    mqtt_.sendError("AciditySensor::OnEnable", error, true);
  }

  clearMeasurements();

  return result == Result::kSuccess;
}

bool AciditySensor::Callback() {
  io_.setStatusLed(true);

  takeMeasurement();

  // Once enough measurements have been taken, stop the task
  if (isMeasurementFull()) {
    disable();
  }

  io_.setStatusLed(false);
  return true;
}

void AciditySensor::OnDisable() {
  io_.disableAnalog(bernd_box::Sensor::kAciditiy);
}

}  // namespace tasks
}  // namespace bernd_box