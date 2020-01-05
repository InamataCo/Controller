#include "acidity_sensor.h"

namespace bernd_box {
namespace tasks {

AciditySensor::AciditySensor(Scheduler* scheduler, Io& io, Mqtt& mqtt)
    : Task(scheduler), io_(io), mqtt_(mqtt), used_sensor_(-1) {
  setIterations(TASK_FOREVER);
  Task::setInterval(std::chrono::milliseconds(default_period_).count());
  clearMeasurements();
}

AciditySensor::~AciditySensor() {}

void AciditySensor::setSensorId(const int sensor_id) {
  clearMeasurements();
  used_sensor_ = sensor_id;
}

bool AciditySensor::isReady() {
  return used_sensor_ > -1;
}

float AciditySensor::getMedianMeasurement() {
  for (const auto& it : samples_) {
    Serial.printf("%f, ", it);
  }
  Serial.printf("\n");

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

  float acidity_ph = analog_v * acidity_factor_v_to_ph - acidity_offset_ph;
  Serial.printf("Acidity: %fV -> %fpH\n", analog_v, acidity_ph);

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
  samples_.fill(NAN);
  sample_index_ = 0;
}

bool AciditySensor::OnEnable() {
  const char* who = __PRETTY_FUNCTION__;
  if(used_sensor_ < 0) {
    mqtt_.sendError(who, "Unknown sensor defined as ID < 0");
    return false;
  }

  // Enable analog sensor and perform a test reading
  Result result = io_.enableAnalog(used_sensor_);
  if (result != Result::kSuccess) {
    String error =
        "Error on enabling analog sensor. Result = " + String(int(result));
    mqtt_.sendError(who, error);
  }

  if (result == Result::kSuccess) {
    float value = io_.readAnalogV(used_sensor_);
    if (std::isnan(value)) {
      String error = "Error performing readAnalogV(" +
                     String(int(used_sensor_)) + "). Returned NAN.";
      mqtt_.sendError(who, error);
      result = Result::kFailure;
    }
  }

  clearMeasurements();

  if (result == Result::kSuccess) {
    mqtt_.send("acidity_sensor_active", "true");
  }

  return result == Result::kSuccess;
}

bool AciditySensor::Callback() {
  // Skip first measurement since it requires a grace period
  if(isFirstIteration()) {
    return true;
  }
  
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
  io_.disableAnalog(used_sensor_);
  mqtt_.send("acidity_sensor_active", "false");
}

}  // namespace tasks
}  // namespace bernd_box
