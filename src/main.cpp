/**
 * \author Moritz Ulmer <moritz.ulmer@posteo.de>
 * \date 2018
 * \copyright Apache License 2.0
 */

#define _TASK_STATUS_REQUEST
#define _TASK_OO_CALLBACKS

#include <TaskScheduler.h>
#include <Timer.h>

// io.h has to be included before mqtt.h
#include "io.h"

#include "configuration.h"
#include "mqtt.h"
#include "network.h"

#include "tasks/acidity_sensor.h"
#include "tasks/analog_sensors.h"
#include "tasks/connectivity.h"
#include "tasks/dallas_temperature.h"
#include "tasks/pump.h"

//----------------------------------------------------------------------------
// Global instances
WiFiClient wifiClient;
Timer timer;
Scheduler scheduler;

bernd_box::Io io;
bernd_box::Mqtt mqtt(wifiClient, bernd_box::client_id, bernd_box::mqtt_server);
bernd_box::Network network(bernd_box::ssid, bernd_box::password);

//----------------------------------------------------------------------------
// TaskScheduler tasks

bernd_box::tasks::Pump pumpTask(&scheduler, io, mqtt);
bernd_box::tasks::CheckConnectivity checkConnectivity(
    &scheduler, network, mqtt, io, bernd_box::wifi_connect_timeout,
    bernd_box::mqtt_connection_attempts);
bernd_box::tasks::DallasTemperature dallasTemperatureTask(&scheduler, io, mqtt);
bernd_box::tasks::AnalogSensors analogSensorsTask(&scheduler, io, mqtt);
bernd_box::tasks::AciditySensor aciditySensorTask(&scheduler, io, mqtt);

//----------------------------------------------------------------------------
// List of available tasks
int8_t readAirSensorsId;
void readAirSensors();

int8_t readLightSensorsId;
void readLightSensors();

namespace measurement_report {
int8_t id;
void callback();
}  // namespace measurement_report

//----------------------------------------------------------------------------
// Setup and loop functions

void setup() {
  Serial.begin(115200);

  checkConnectivity.now();
  checkConnectivity.setInterval(std::chrono::milliseconds(100).count());
  checkConnectivity.enable();

  // Try to configure the IO devices, else restart
  if (io.init() != bernd_box::Result::kSuccess) {
    Serial.println("IO: Initialization failed. Restarting\n");
    ESP.restart();
  }

  pumpTask.setDuration(std::chrono::seconds(20));
  pumpTask.enable();

  analogSensorsTask.setInterval(std::chrono::milliseconds(1000).count());
  analogSensorsTask.enable();

  // readAirSensorsId = timer.every(10000, readAirSensors);
  // readLightSensorsId = timer.every(10000, readLightSensors);
  // update_dallas_temperature_sample::id =
  //     timer.every(1000, update_dallas_temperature_sample::callback);
  // measurement_report::id = timer.every(1000, measurement_report::callback);
}

// Update both schedulers
void loop() {
  timer.update();
  scheduler.execute();
}

//----------------------------------------------------------------------------
// Implementations of available tasks
// Reads, prints and then sends all air sensor parameters
void readAirSensors() {
  io.setStatusLed(true);

  for (const auto& bme : io.bme280s_) {
    float value = io.readBme280Air(bme.first);
    Serial.printf("The %s is %f %s\n", bme.second.name.c_str(), value,
                  bme.second.unit.c_str());
    mqtt.send(bme.second.name.c_str(), value);
  }

  io.setStatusLed(false);
}

// Reads, prints and then sends all light sensors
void readLightSensors() {
  io.setStatusLed(true);

  for (const auto& max44009 : io.max44009s_) {
    float value = io.readMax44009Light(max44009.first);
    Serial.printf("Ambient brightness (ID: %u) is %f %s\n",
                  static_cast<int>(max44009.first), value,
                  max44009.second.unit.c_str());
    mqtt.send(max44009.second.name.c_str(), value);
  }

  io.setStatusLed(false);
}

namespace measurement_report {

bool is_state_finished = true;
bool is_report_finished = true;
std::chrono::milliseconds start_time;

std::chrono::seconds pump_duration(20);

std::chrono::milliseconds tds_start_time;
std::chrono::seconds tds_duration(20);

std::chrono::milliseconds temperature_request_time;

std::chrono::milliseconds turbidity_start_time;
std::chrono::seconds turbidity_duration(10);

bernd_box::Sensor report_list[] = {
    bernd_box::Sensor::kUnknown, bernd_box::Sensor::kPump,
    bernd_box::Sensor::kAciditiy, bernd_box::Sensor::kUnknown};

int report_index = 0;

void callback() {
  Serial.printf("Measurement report. sensor_task: %i\n",
                int(report_list[report_index]));

  switch (report_list[report_index]) {
    case bernd_box::Sensor::kUnknown: {
      // Initial state. Select first state to go to
      if (is_report_finished) {
        Serial.println("Starting measurement report");
        start_time = std::chrono::milliseconds(millis());
        is_state_finished = true;
        is_report_finished = false;
        report_index++;
      } else {
        // TODO: End report
        timer.stop(id);
        Serial.printf("Measurement report finished after %llus\n",
                      std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::milliseconds(millis()) - start_time)
                          .count());
        report_index = 0;
      }
    } break;
    case bernd_box::Sensor::kPump: {
      if (is_state_finished) {
        Serial.println("Starting pumping");
        is_state_finished = false;

        pumpTask.setDuration(pump_duration);
        pumpTask.enable();
      }

      if (!pumpTask.isEnabled()) {
        Serial.println("Finished pumping");

        is_state_finished = true;
        report_index++;
      }
    } break;
    case bernd_box::Sensor::kTotalDissolvedSolids: {
      const bernd_box::Sensor tds_id = bernd_box::Sensor::kTotalDissolvedSolids;
      if (is_state_finished) {
        Serial.println("Starting total dissolved solids measurement");
        is_state_finished = false;

        io.enableAnalog(tds_id);
        dallasTemperatureTask.enable();

        tds_start_time = std::chrono::milliseconds(millis());
      }

      float raw_analog = io.readAnalog(tds_id);
      float analog_v =
          raw_analog * io.analog_reference_v_ / io.analog_raw_range_;
      float temperature_c = dallasTemperatureTask.getLastSample().value;
      float temperature_coefficient = 1 + 0.02 * (temperature_c - 25);
      float compensation_v = analog_v / temperature_coefficient;
      float tds =
          (133.42 * std::pow(compensation_v, 3) -
           255.86 * std::pow(compensation_v, 2) + 857.39 * compensation_v) *
          0.5;
      Serial.printf(
          "TDS = %fmg/L, Compensation = %f, TempCoef = %f, °C = %f, Analog V = "
          "%f\n",
          tds, compensation_v, temperature_coefficient, temperature_c,
          analog_v);

      // Exit condition
      if (tds_duration < std::chrono::milliseconds(millis()) - tds_start_time) {
        Serial.println("Finished pumping");

        dallasTemperatureTask.disable();
        io.disableAnalog(tds_id);

        is_state_finished = true;
        report_index++;
      }
    } break;
    case bernd_box::Sensor::kAciditiy: {
      // Execute once at start of task
      if (is_state_finished) {
        Serial.println("Starting acidity measurement");

        is_state_finished = false;

        aciditySensorTask.setInterval(1000);
        aciditySensorTask.enable();
      }

      // Exit condition, once enough samples have been collected
      if (aciditySensorTask.isMeasurementFull()) {
        Serial.println("Finishing acidity measurement");

        float acidity_ph = aciditySensorTask.getMedianMeasurement();

        Serial.printf("Acidity is %f pH\n", acidity_ph);
        mqtt.send("acidity_ph", acidity_ph);

        is_state_finished = true;
        report_index++;
      }

    } break;
    case bernd_box::Sensor::kWaterTemperature: {
      // Start the update dallas temperature thread
      if (is_state_finished) {
        temperature_request_time = std::chrono::milliseconds(millis());

        dallasTemperatureTask.enable();
        is_state_finished = false;
      }

      // End once an update has been saved
      const bernd_box::Measurement& measurement =
          dallasTemperatureTask.getLastSample();
      if (measurement.timestamp > temperature_request_time) {
        dallasTemperatureTask.disable();

        Serial.printf("Temperature is %f °C\n", measurement.value);
        mqtt.send("water_temperature_c", measurement.value);

        is_state_finished = true;
        report_index++;
        // TODO: check the sensor ID
      }

    } break;
    case bernd_box::Sensor::kTurbidity: {
      if (is_state_finished) {
        turbidity_start_time = std::chrono::milliseconds(millis());

        is_state_finished = false;
      }

      float turbidity_v = io.readAnalog(bernd_box::Sensor::kTurbidity) *
                          io.analog_reference_v_ / io.analog_raw_range_;
      Serial.printf("Turbidity is %fV\n", turbidity_v);

      // End once an update has been saved
      if (turbidity_duration <
          std::chrono::milliseconds(millis()) - turbidity_start_time) {
        Serial.printf("Turbidity is %fV\n", turbidity_v);
        mqtt.send(io.adcs_.at(bernd_box::Sensor::kTurbidity).name, turbidity_v);

        is_state_finished = true;
        report_index++;
        // TODO: check the sensor ID
      }

    } break;
    default: {
      mqtt.sendError("Measurement Report",
                     "Transition to unhandled state" +
                         String(int(report_list[report_index])),
                     true);

      report_index++;
      is_report_finished = true;
    }
  }
}
}  // namespace measurement_report
