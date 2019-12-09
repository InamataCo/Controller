#include "tasks/dallas_temperature.h"

namespace bernd_box {
namespace tasks {

DallasTemperature::DallasTemperature(Scheduler* scheduler, Io& io, Mqtt& mqtt)
    : Task(scheduler), io_(io), mqtt_(mqtt) {
  setIterations(TASK_FOREVER);
  samples_.fill({NAN, std::chrono::seconds::zero(), Sensor::kUnknown});
}

DallasTemperature::~DallasTemperature() {}

Measurement DallasTemperature::getLastSample() {
  return samples_[sample_index_];
}

void DallasTemperature::pushSample(const float temperature_c, Sensor sensorId) {
  sample_index_++;

  if (sample_index_ >= samples_.size()) {
    sample_index_ = 0;
  }

  samples_[sample_index_] = {temperature_c, std::chrono::milliseconds(millis()),
                             sensorId};
  has_new_samples_ = true;
}

void DallasTemperature::clearSamples() {
  for (auto& sample : samples_) {
    sample = {NAN, std::chrono::milliseconds(0), Sensor::kUnknown};
  }
  sample_index_ = 0;
  has_new_samples_ = false;
}

bool DallasTemperature::OnEnable() {
  mqtt_.send("dallas_temperature_sensor_active", "true");
  clearSamples();

  return true;
}

bool DallasTemperature::Callback() {
  io_.setStatusLed(true);

  int next_activation_ms = 0;
  bernd_box::Result result = bernd_box::Result::kSuccess;

  for (const auto& dallas : io_.dallases_) {
    if (is_request_sent == false) {
      int wait_ms = 0;
      result = io_.requestDallasTemperatureUpdate(dallas.second, wait_ms);
      if (result == bernd_box::Result::kSuccess) {
        if (wait_ms > next_activation_ms) {
          next_activation_ms = wait_ms;
          is_request_sent = true;
        }
      }
    } else {
      float temperature_c = NAN;

      result = io_.readDallasTemperature(dallas.second, true, temperature_c);
      if (result == bernd_box::Result::kSuccess) {
        pushSample(temperature_c, dallas.first);
        Serial.printf("Temperature of %s = %.2f %s\n",
                      dallas.second.name.c_str(), temperature_c,
                      dallas.second.unit.c_str());
        next_activation_ms =
            std::chrono::milliseconds(sleep_between_measurements_).count();
        is_request_sent = false;
      }
    }

    if (result != bernd_box::Result::kSuccess) {
      String error = String("Result: ") + String(int(result)) +
                     String(", is_request_sent: ") + String(is_request_sent);
      mqtt_.sendError("updateDallasTemperatureSample", error, true);
    }
  }

  setInterval(next_activation_ms);

  io_.setStatusLed(false);

  return true;
}

void DallasTemperature::OnDisable() {
    mqtt_.send("dallas_temperature_sensor_active", "false");
}

}  // namespace tasks
}  // namespace bernd_box
