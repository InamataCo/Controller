/**
 * \author Moritz Ulmer <moritz.ulmer@posteo.de>
 * \date 2018
 * \copyright Apache License 2.0
 */

#include "Arduino.h"
#include "configuration.h"
#include "io.h"
#include "mqtt.h"
#include "network.h"
#include "utils/setupNode.h"

// tasks/task.h before TaskScheduler as it sets the TaskScheduler defines
// clang-format off
#include "tasks/task.h"
#include <TaskScheduler.h>
// clang-format on

#include "tasks/acidity_sensor.h"
#include "tasks/air_sensors.h"
#include "tasks/analog_sensors.h"
#include "tasks/connectivity.h"
#include "tasks/dallas_temperature.h"
#include "tasks/dissolved_oxygen_sensor.h"
#include "tasks/light_sensors.h"
#include "tasks/measurement_protocol.h"
#include "tasks/pump.h"
#include "tasks/system_monitor.h"

//----------------------------------------------------------------------------
// Global instances
WiFiClient wifiClient;
Scheduler scheduler;

bernd_box::Io io;
bernd_box::Mqtt mqtt(wifiClient);
bernd_box::Network network(bernd_box::ssid, bernd_box::password);

//----------------------------------------------------------------------------
// TaskScheduler tasks and report list
std::vector<bernd_box::tasks::ReportItem> report_list{
    {bernd_box::tasks::Action::kPump, std::chrono::seconds(20)},
    {bernd_box::tasks::Action::kWaterTemperature},
    {bernd_box::tasks::Action::kPump, std::chrono::seconds(20)},
    {bernd_box::tasks::Action::kDissolvedOxygen},
    {bernd_box::tasks::Action::kPump, std::chrono::seconds(20)},
    {bernd_box::tasks::Action::kTotalDissolvedSolids, std::chrono::seconds(10)},
    {bernd_box::tasks::Action::kPump, std::chrono::seconds(20)},
    {bernd_box::tasks::Action::kAcidity},
    {bernd_box::tasks::Action::kPump, std::chrono::seconds(20)},
    {bernd_box::tasks::Action::kTurbidity, std::chrono::seconds(10)},
    {bernd_box::tasks::Action::kSleep, std::chrono::minutes(15)},
};

bernd_box::tasks::Pump pumpTask(&scheduler, io, mqtt);
bernd_box::tasks::CheckConnectivity checkConnectivity(
    &scheduler, network, mqtt, io, bernd_box::wifi_connect_timeout,
    bernd_box::mqtt_connection_attempts);
bernd_box::tasks::DallasTemperature dallasTemperatureTask(&scheduler, io, mqtt);
bernd_box::tasks::AnalogSensors analogSensorsTask(&scheduler, io, mqtt);
bernd_box::tasks::AciditySensor aciditySensorTask(&scheduler, io, mqtt);
bernd_box::tasks::LightSensors lightSensorsTask(&scheduler, io, mqtt);
bernd_box::tasks::AirSensors airSensorsTask(&scheduler, io, mqtt);
bernd_box::tasks::DissolvedOxygenSensor dissolvedOxygenSensorTask(&scheduler,
                                                                  io, mqtt);
bernd_box::tasks::MeasurementProtocol measurementProtocol(
    &scheduler, mqtt, io, report_list, pumpTask, dallasTemperatureTask,
    dissolvedOxygenSensorTask, aciditySensorTask);
bernd_box::tasks::SystemMonitor systemMonitorTask(&scheduler, mqtt);

//----------------------------------------------------------------------------
// Setup and loop functions

// void mqttCallback(char*, uint8_t*, unsigned int) {
//   Serial.println(F("Enabling pump"));
//   pumpTask.enable();
// }

void setup() {
  io.setStatusLed(true);
  if (!bernd_box::setupNode()) {
    Serial.println(F("Node setup failed. Restarting"));
    delay(1000);
    ESP.restart();
  }

  checkConnectivity.enable();
  systemMonitorTask.enable();
  // systemMonitorTask.forceNextIteration();
  
  // mqtt.client_.setCallback(mqttCallback);
  // mqtt.client_.subscribe("pump");

  // Try to configure the IO devices, else restart
  if (io.init() != bernd_box::Result::kSuccess) {
    Serial.println(F("IO: Initialization failed. Restarting"));
    delay(1000);
    ESP.restart();
  }

  // pumpTask.enable();
  // pinMode(32, INPUT_PULLUP);

  // measurementProtocol.enable();

  checkConnectivity.isSetup_ = true;
  io.setStatusLed(false);
}

void loop() {
  scheduler.execute();
  // if (!digitalRead(32)) {
  //   Serial.println("button!");
  //   mqtt.send("button", true);
  //   pumpTask.enable();
  // }
}
