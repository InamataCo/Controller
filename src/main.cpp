#include <Arduino.h>
#include <TaskScheduler.h>

#include "managers/services.h"
// #include "utils/ota.h"
#include "utils/setup_node.h"

//----------------------------------------------------------------------------
// Global instances
bernd_box::Services services;
Scheduler& scheduler = services.getScheduler();

//----------------------------------------------------------------------------
// Setup and loop functions
void setup() {
  bool success = bernd_box::setupNode(services);
  if (success) {
    Serial.println(F("Setup finished"));
  } else {
    Serial.println(F("Node setup failed. Restarting in 10s"));
    delay(10000);
    ESP.restart();
  }
  // sdg::setup_ota("bernd_box_a", "sdg");
}

void loop() {
  // sdg::handle_ota();
  scheduler.execute();
}
