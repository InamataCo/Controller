/**
 * \author Moritz Ulmer <moritz.ulmer@posteo.de>
 * \date 2018
 * \copyright Apache License 2.0
 */

#include "Arduino.h"

// #include "utils/setupNode.h"
#include "configuration.h"
#include "io.h"
#include "mqtt.h"
#include "network.h"

// tasks/task.h before TaskScheduler as it sets the TaskScheduler defines
#include "tasks/task.h"

#include <TaskScheduler.h>

#include "tasks/acidity_sensor.h"
#include "tasks/air_sensors.h"
#include "tasks/analog_sensors.h"
#include "tasks/connectivity.h"
#include "tasks/dallas_temperature.h"
#include "tasks/dissolved_oxygen_sensor.h"
#include "tasks/light_sensors.h"
#include "tasks/measurement_protocol.h"
#include "tasks/pump.h"

//----------------------------------------------------------------------------
// Global instances
WiFiClient wifiClient;
Scheduler scheduler;

bernd_box::Io io;
bernd_box::Mqtt mqtt(wifiClient, bernd_box::client_id, bernd_box::mqtt_server);
bernd_box::Network network(bernd_box::ssid, bernd_box::password);

//----------------------------------------------------------------------------
// TaskScheduler tasks and report list
std::vector<bernd_box::tasks::ReportItem> report_list{
    {bernd_box::tasks::Action::kPump, std::chrono::seconds(20)},
    {bernd_box::tasks::Action::kWaterTemperature},
    {bernd_box::tasks::Action::kPump, std::chrono::seconds(20)},
    {bernd_box::tasks::Action::kDissolvedOxygen},
    {bernd_box::tasks::Action::kPump, std::chrono::seconds(20)},
    {bernd_box::tasks::Action::kTotalDissolvedSolids,
     std::chrono::seconds(10)},
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

//----------------------------------------------------------------------------
// Setup and loop functions
void setup() {
  Serial.begin(115200);
  // setupNode();

  checkConnectivity.now();
  checkConnectivity.enable();

  // Try to configure the IO devices, else restart
  if (io.init() != bernd_box::Result::kSuccess) {
    Serial.println("IO: Initialization failed. Restarting\n");
    ESP.restart();
  }

  // measurementProtocol.enable();
}

void loop() {
  scheduler.execute();
}
