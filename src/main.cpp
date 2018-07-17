#include <Arduino.h>

#include "configuration.h"
#include "wifi.h"

bernd_box::Wifi wifi(ssid, password);

// Function to reset the program
void (*reset)(void) = 0;

void setup() {
  Serial.begin(115200);

  // Try to connect to Wifi within 10 seconds, else reset program
  if (wifi.connect(std::chrono::seconds(10)) == false) {
    reset();
  }
  wifi.printState();

  // Setup ADCs
}

void loop() {
  Serial.println("/////////////////////////");
  delay(1000);

  // Read and print the values of the six ADC pins
  for (int i = 32; i < 40; i++) {
    if(!(i == 37 || i == 38)) {
      uint16_t analog_value = analogRead(i);

      Serial.print("Pin ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(analog_value);

      delay(100);
    }
  }
}
