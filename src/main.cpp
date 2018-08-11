/**
 * \author Moritz Ulmer <moritz.ulmer@posteo.de>
 * \date 2018
 * \copyright Apache License 2.0
 */

#include <Timer.h>

// io.h has to be included before mqtt.h
#include "io.h"

#include "configuration.h"
#include "mqtt.h"
#include "wifi.h"

WiFiClient wifiClient;
Timer timer;

bernd_box::Io io;
bernd_box::Mqtt mqtt(wifiClient, bernd_box::client_id, bernd_box::mqtt_server);
bernd_box::Wifi wifi(bernd_box::ssid, bernd_box::password);

int8_t checkConnectivityId;
void checkConnectivity();

int8_t readAnalogSensorsId;
void readAnalogSensors();

int8_t updateAciditySensorId;
void updateAciditySensor();

// If not connected to WiFi, attempt to reconnect. Finally reboot
void checkConnectivity() {
  io.setStatusLed(true);

  if (!wifi.isConnected()) {
    Serial.println("WiFi: Disconnected. Attempting to reconnect");
    if (wifi.connect(bernd_box::wifi_connect_timeout) == false) {
      Serial.printf("WiFi: Could not connect to %s. Restarting\n",
                    bernd_box::ssid);
      ESP.restart();
    }
  }

  // If not connected to an MQTT broker, attempt to reconnect. Else reboot
  if (!mqtt.isConnected()) {
    Serial.println("MQTT: Disconnected. Attempting to reconnect");
    if (mqtt.connect(bernd_box::connection_attempts) == false) {
      Serial.println("MQTT: Could not connect to broker. Restarting\n");
      ESP.restart();
    }
  }

  io.setStatusLed(false);
}

// Read and then print the analog sensors
void readAnalogSensors() {
  io.setStatusLed(true);

  // Start a measurement series for the acidity sensor
  updateAciditySensorId = timer.every(30, updateAciditySensor);

  // Read all sensors. Then print and send them over MQTT
  Serial.printf("\n%-10s|%-4s|%-15s|%s\n", "Sensor", "Pin", "Value", "Unit");
  Serial.printf("----------|----|---------------|----\n");
  for (auto& it : io.adcs_) {
    float value = io.readAnalog(it.first);

    Serial.printf("%-10s|%-4i|%-15f|%s\n", it.second.name.c_str(),
                  it.second.pin_id, value, it.second.unit.c_str());
    mqtt.send(it.second.name.c_str(), value);
  }

  io.setStatusLed(false);
}

void updateAciditySensor() {
  io.setStatusLed(true);

  io.takeAcidityMeasurement();

  // Once enough measurements have been taken, stop the task and send
  if (io.isAcidityMeasurementFull()) {
    timer.stop(updateAciditySensorId);

    float measurement = io.getMedianAcidityMeasurement();
    const bernd_box::AdcSensor& sensor =
        io.adcs_.find(bernd_box::Sensor::kAciditiy)->second;

    Serial.printf("Median acidity value is %f %s\n", measurement,
                  sensor.unit.c_str());
    mqtt.send(sensor.name, measurement);
  }

  io.setStatusLed(false);
}

void setup() {
  Serial.begin(115200);

  // Try to connect to Wifi within wifi_connect_timeout, else restart
  io.setStatusLed(true);
  if (wifi.connect(bernd_box::wifi_connect_timeout) == false) {
    Serial.printf("WiFi: Could not connect to %s. Restarting\n",
                  bernd_box::ssid);
    ESP.restart();
  }
  wifi.printState();

  // Try to connect to the MQTT broker 3 times, else restart
  io.setStatusLed(false);
  if (mqtt.connect(bernd_box::connection_attempts) == false) {
    Serial.println("MQTT: Could not connect to broker. Restarting\n");
    ESP.restart();
  }

  // Try to configure the IO devices, else restart
  if (io.init()) {
    Serial.println("IO: Initialization failed. Restarting\n");
    ESP.restart();
  }

  checkConnectivityId = timer.every(1000, checkConnectivity);
  readAnalogSensorsId = timer.every(1000, readAnalogSensors);
}

void loop() { timer.update(); }
