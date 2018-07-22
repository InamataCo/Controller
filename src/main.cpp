/**
 * \author Moritz Ulmer <moritz.ulmer@posteo.de>
 * \date 2018
 * \copyright Apache License 2.0
 */

#include "adc.h"
#include "configuration.h"
#include "mqtt.h"
#include "wifi.h"

WiFiClient wifiClient;

bernd_box::Wifi wifi(bernd_box::ssid, bernd_box::password);
bernd_box::Adc adc;
bernd_box::Mqtt mqtt(wifiClient, bernd_box::client_id, bernd_box::mqtt_server);

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);

  // Try to connect to Wifi within wifi_connect_timeout, else restart
  digitalWrite(2, HIGH);
  if (wifi.connect(bernd_box::wifi_connect_timeout) == false) {
    Serial.printf("WiFi: Could not connect to %s. Restarting\n",
                  bernd_box::ssid);
    ESP.restart();
  }
  wifi.printState();

  // Try to connect to the MQTT broker 3 times, else restart
  digitalWrite(2, LOW);
  if (mqtt.connect(3) == false) {
    Serial.println("MQTT: Could not connect to broker. Restarting\n");
    ESP.restart();
  }
}

void loop() {
  // Turn the blue on-board LED on during the loop
  digitalWrite(2, HIGH);

  // If not connected to WiFi, attempt to reconnect. Finally reboot
  if (!wifi.isConnected()) {
    Serial.println("WiFi: Disconnected. Attempting to reconnect");
    if (wifi.connect(bernd_box::wifi_connect_timeout) == false) {
      Serial.printf("WiFi: Could not connect to %s. Restarting\n",
                    bernd_box::ssid);
      ESP.restart();
    }
  }

  // Read all sensors. Then print and send them over MQTT
  Serial.printf("\n%-10s|%-4s|%-15s|%s\n", "Sensor", "Pin", "Value", "Unit");
  Serial.printf("----------|----|---------------|----\n");
  for (uint i = 0; i < adc.getSensorCount(); i++) {
    float value = adc.read(i);

    Serial.printf("%-10s|%-4i|%-15f|%s\n", adc.getSensorName(i).c_str(),
                  adc.getSensorPin(i), value, adc.getSensorUnit(i).c_str());
    mqtt.send(adc.getSensorName(i), value);
    delay(1);
  }

  // Turn the blue on-board LED off before sleeping
  digitalWrite(2, LOW);

  delay(1000);
}
