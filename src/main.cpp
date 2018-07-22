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
  digitalWrite(2, HIGH);

  // Try to connect to Wifi within 10 seconds, else reset chip
  if (wifi.connect(std::chrono::seconds(20)) == false) {
    Serial.printf("WiFi could not connect to %s. Restarting\n",
                  bernd_box::ssid);
    ESP.restart();
  }
  wifi.printState();
  digitalWrite(2, LOW);

  // Try to connect to the MQTT broker
  if (mqtt.connect(3) == false) {
    Serial.println("Could not connect to the MQTT broker. Restarting\n");
    ESP.restart();
  }
}

void loop() {
  // Turn the blue on-board LED on during the loop
  digitalWrite(2, HIGH);

  if (!wifi.isConnected()) {
    Serial.println("Disconnected from WiFi. Restarting");
    ESP.restart();
  }

  Serial.println("/////////////////////////");
  for (uint i = 0; i < adc.getSensorCount(); i++) {
    float value = adc.read(i);

    Serial.printf("Adc class read(%i) = %f\n", i, value);
    mqtt.send(adc.getSensorName(i), value);
    delay(1);
  }

  // Turn the blue on-board LED off before sleeping
  digitalWrite(2, LOW);

  delay(1000);
}
