/**
 * \author Moritz Ulmer <moritz.ulmer@posteo.de>
 * \date 2018
 * \copyright Apache License 2.0
 */

// io.h has to be included before mqtt.h
#include "io.h"

#include "configuration.h"
#include "mqtt.h"
#include "wifi.h"

WiFiClient wifiClient;

bernd_box::Io io;
bernd_box::Mqtt mqtt(wifiClient, bernd_box::client_id, bernd_box::mqtt_server);
bernd_box::Wifi wifi(bernd_box::ssid, bernd_box::password);

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
}

void loop() {
  // Turn the blue on-board LED on during the loop
  io.setStatusLed(true);

  // If not connected to WiFi, attempt to reconnect. Finally reboot
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

  // Read all sensors. Then print and send them over MQTT
  Serial.printf("\n%-10s|%-4s|%-15s|%s\n", "Sensor", "Pin", "Value", "Unit");
  Serial.printf("----------|----|---------------|----\n");
  for (auto& it : io.adcs_) {
    float value = io.readAnalog(it.first);

    Serial.printf("%-10s|%-4i|%-15f|%s\n", it.second.name.c_str(),
                  it.second.pin_id, value, it.second.unit.c_str());
    mqtt.send(it.second.name.c_str(), value);
    delay(1);
  }

  // Turn the blue on-board LED off before sleeping
  io.setStatusLed(false);
  delay(1000);
}
