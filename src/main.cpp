/**
 * \author Moritz Ulmer <moritz.ulmer@posteo.de>
 * \date 2018
 * \copyright Apache License 2.0
 */

#include "adc.h"
#include "configuration.h"
#include "wifi.h"

bernd_box::Wifi wifi(bernd_box::ssid, bernd_box::password);
bernd_box::Adc adc;

void setup() {
  Serial.begin(115200);

  // Try to connect to Wifi within 10 seconds, else reset chip
  if (wifi.connect(std::chrono::seconds(10)) == false) {
    Serial.println("Could not connect to wifi. Restarting");
    ESP.restart();
  }
  wifi.printState();

  // Setup ADCs
}

void loop() {
  Serial.println("/////////////////////////");
  for (uint i = 0; i < adc.getSensorCount(); i++) {
    Serial.printf("Adc class read(%i) = %f\n", i, adc.read(i));
    delay(1);
  }

  delay(1000);
}
