#include "measurement_protocol.h"

namespace bernd_box {
namespace tasks {

MeasurementProtocol::MeasurementProtocol(
    Scheduler* scheduler, Mqtt& mqtt, Io& io,
    std::vector<bernd_box::tasks::ReportItem>& report_list,
    DallasTemperature& dallas_temperature_task,
    DissolvedOxygenSensor& dissolved_oxygen_sensor_task,
    AciditySensor& acidity_sensor_task)
    : Task(scheduler),
      mqtt_(mqtt),
      io_(io),
      report_list_(report_list),
      dallas_temperature_task_(dallas_temperature_task),
      dissolved_oxygen_sensor_task_(dissolved_oxygen_sensor_task),
      acidity_sensor_task_(acidity_sensor_task) {
  Task::setIterations(TASK_FOREVER);
  Task::setInterval(std::chrono::milliseconds(default_period_).count());
}

MeasurementProtocol::~MeasurementProtocol() {}

bool MeasurementProtocol::OnEnable() {
  const char* who = __PRETTY_FUNCTION__;
  if(!acidity_sensor_task_.isReady()) {
    mqtt_.sendError(who, String(F("Acidity sensor task is not ready")));
    return false;
  }
  if(!dissolved_oxygen_sensor_task_.isReady()) {
    mqtt_.sendError(who, String(F("Dissolved oxygen sensor task is not ready")));
    return false;
  }

  Serial.println("Starting measurement report");
  mqtt_.send("measurement_report_active", "true");

  start_time_ = std::chrono::milliseconds(millis());
  report_index_ = 0;
  is_report_item_enabled = false;

  return true;
}

bool MeasurementProtocol::Callback() {
  Serial.printf("Running measurement protocol action: %s\n",
                getCurrentActionName());

  if (report_index_ < report_list_.size()) {
    switch (report_list_[report_index_].action) {
      case Action::kPump: {
        if (!is_report_item_enabled) {
          is_report_item_enabled = true;
        }

      } break;
      case Action::kWaterTemperature: {
        if (!is_report_item_enabled) {
          dallas_temperature_task_.enable();
          is_report_item_enabled = true;
        }

        if (dallas_temperature_task_.has_new_samples_) {
          const bernd_box::Measurement& measurement =
              dallas_temperature_task_.getLastSample();
          Serial.printf("Temperature is %f °C\n", measurement.value);
          mqtt_.send("water_temperature_c", measurement.value);

          dallas_temperature_task_.disable();

          report_index_++;
          is_report_item_enabled = false;
        }
      } break;
      case Action::kDissolvedOxygen: {
        if (!is_report_item_enabled) {
          dissolved_oxygen_sensor_task_.setInterval(1000);
          dissolved_oxygen_sensor_task_.enable();
          is_report_item_enabled = true;
        }

        // Exit condition, once enough samples have been collected
        if (dissolved_oxygen_sensor_task_.isMeasurementFull()) {
          Serial.println("Finishing dissolved oxygen measurement");

          float dissolved_oxygen_percent =
              dissolved_oxygen_sensor_task_.getLastMeasurement();

          Serial.printf("Dissolved oxygen is %fmg/L\n",
                        dissolved_oxygen_percent);
          mqtt_.send("dissolved_oxygen_mg_L", dissolved_oxygen_percent);

          report_index_++;
          is_report_item_enabled = false;
        }
      } break;
      // case Action::kTotalDissolvedSolids: {
      //   const bernd_box::Sensor tds_id =
      //       bernd_box::Sensor::kTotalDissolvedSolids;
      //   if (!is_report_item_enabled) {
      //     mqtt_.send("total_dissolved_solids_sensor_active", "true");
      //     is_report_item_enabled = true;

      //     io_.enableAnalog(tds_id);
      //     dallas_temperature_task_.enable();
      //     pump_task_.setDuration(report_list_[report_index_].duration);
      //     pump_task_.enable();
      //   }

      //   float raw_analog = io_.readAnalog(tds_id);
      //   float analog_v =
      //       raw_analog * io_.analog_reference_v_ / io_.analog_raw_range_;
      //   float temperature_c = dallas_temperature_task_.getLastSample().value;
      //   float temperature_coefficient = 1 + 0.02 * (temperature_c - 25);
      //   float compensation_v = analog_v / temperature_coefficient;
      //   float tds =
      //       (133.42 * std::pow(compensation_v, 3) -
      //        255.86 * std::pow(compensation_v, 2) + 857.39 * compensation_v) *
      //       0.5;
      //   Serial.printf(
      //       "TDS = %fmg/L, Compensation = %f, TempCoef = %f, °C = %f, Analog V "
      //       "= "
      //       "%f\n",
      //       tds, compensation_v, temperature_coefficient, temperature_c,
      //       analog_v);

      //   // Exit condition. Replace with tds task check when implemented
      //   if (!pump_task_.isEnabled()) {
      //     Serial.println("Finished pumping");

      //     mqtt_.send("total_dissolved_solids_mg_per_L", tds);
      //     mqtt_.send("total_dissolved_solids_sensor_active", "false");

      //     dallas_temperature_task_.disable();
      //     io_.disableAnalog(tds_id);

      //     report_index_++;
      //     is_report_item_enabled = false;
      //   }
      // } break;
      case Action::kAcidity: {
        // Execute once at start of task
        if (!is_report_item_enabled) {
          Serial.println("Starting acidity measurement");

          is_report_item_enabled = true;

          acidity_sensor_task_.setInterval(1000);
          acidity_sensor_task_.enable();
        }

        // Exit condition, once enough samples have been collected
        if (acidity_sensor_task_.isMeasurementFull()) {
          Serial.println("Finishing acidity measurement");

          float acidity_ph = acidity_sensor_task_.getMedianMeasurement();

          Serial.printf("Acidity is %f pH\n", acidity_ph);
          mqtt_.send("acidity_ph", acidity_ph);

          report_index_++;
          is_report_item_enabled = false;
        }
      } break;
      // case Action::kTurbidity: {
      //   if (!is_report_item_enabled) {
      //     pump_task_.setDuration(report_list_[report_index_].duration);
      //     pump_task_.enable();
      //     is_report_item_enabled = true;
      //   }

      //   float turbidity_v = io_.readAnalog(bernd_box::Sensor::kTurbidity) *
      //                       io_.analog_reference_v_ / io_.analog_raw_range_;
      //   Serial.printf("Turbidity is %fV\n", turbidity_v);

      //   // End once an update has been saved
      //   if (!pump_task_.isEnabled()) {
      //     Serial.printf("Turbidity is %fV\n", turbidity_v);
      //     mqtt_.send(io_.adcs_.at(bernd_box::Sensor::kTurbidity).name,
      //                turbidity_v);

      //     report_index_++;
      //     is_report_item_enabled = false;
      //     // TODO: check the sensor ID
      //   }
      // } break;
      case Action::kSleep: {
        if (!is_report_item_enabled) {
          Serial.printf(
              "Starting sleep for %llus",
              std::chrono::milliseconds(report_list_[report_index_].duration)
                      .count() /
                  1000);
          mqtt_.send("measurement_report_active", "false");

          sleep_start_time_ = std::chrono::milliseconds(millis());
          is_report_item_enabled = true;
        }
        if (report_list_[report_index_].duration <
            std::chrono::milliseconds(millis()) - sleep_start_time_) {
          Serial.println("Finished sleeping");

          is_report_item_enabled = false;
          report_index_++;
        }
      } break;
      default: {
        mqtt_.sendError("Measurement Report",
                        "Transition to unhandled state" +
                            String(int(report_list_[report_index_].action)),
                        true);

        // End the measurement protocol on error
        disable();
      }
    }
  } else {
    // Report list has been processed
    disable();
  }
  return true;
}

void MeasurementProtocol::OnDisable() {
  Serial.printf("Measurement report finished after %llums\n",
                (std::chrono::milliseconds(millis()) - start_time_).count());
  mqtt_.send("measurement_report_active", "false");
}

const char* MeasurementProtocol::getCurrentActionName() {
  return bernd_box::tasks::actionName.at(report_list_[report_index_].action);
}

}  // namespace tasks
}  // namespace bernd_box
