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
  pinMode(bernd_box::status_led, OUTPUT);

  // Try to connect to Wifi within wifi_connect_timeout, else restart
  digitalWrite(bernd_box::status_led, HIGH);
  if (wifi.connect(bernd_box::wifi_connect_timeout) == false) {
    Serial.printf("WiFi: Could not connect to %s. Restarting\n",
                  bernd_box::ssid);
    ESP.restart();
  }
  wifi.printState();

  // Try to connect to the MQTT broker 3 times, else restart
  digitalWrite(bernd_box::status_led, LOW);
  if (mqtt.connect(bernd_box::connection_attempts) == false) {
    Serial.println("MQTT: Could not connect to broker. Restarting\n");
    ESP.restart();
  }
}

void loop() {
  // Turn the blue on-board LED on during the loop
  digitalWrite(bernd_box::status_led, HIGH);

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
  for (uint i = 0; i < adc.getSensorCount(); i++) {
    float value = adc.read(i);

    Serial.printf("%-10s|%-4i|%-15f|%s\n", adc.getSensorName(i).c_str(),
                  adc.getSensorPin(i), value, adc.getSensorUnit(i).c_str());
    mqtt.send(adc.getSensorName(i), value);
    delay(1);
  }

  // Test sending boolean and negative integer values
  mqtt.send("boolean", true);
  mqtt.send("integer", -11);

  // Turn the blue on-board LED off before sleeping
  digitalWrite(bernd_box::status_led, LOW);
  delay(1000);
}
