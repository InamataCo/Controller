#include <Arduino.h>
#include <TaskScheduler.h>

#include "managers/services.h"
#include "utils/ota.h"
#include "tasks/connectivity/connectivity.h"
#include "tasks/system_monitor/system_monitor.h"
#include "utils/setupNode.h"

//----------------------------------------------------------------------------
// Global instances
Scheduler& scheduler = bernd_box::Services::getScheduler();
bernd_box::tasks::connectivity::CheckConnectivity checkConnectivity(&scheduler);
bernd_box::tasks::system_monitor::SystemMonitor systemMonitorTask(&scheduler);

//----------------------------------------------------------------------------
// Setup and loop functions
void setup() {
  if (!bernd_box::setupNode()) {
    Serial.println(F("Node setup failed. Restarting in 10s"));
    delay(10000);
    ESP.restart();
  }

  checkConnectivity.enable();
  systemMonitorTask.enable();

  sdg::setup_ota("bernd_box_a", "sdg");
}

void loop() {
  sdg::handle_ota();
  scheduler.execute();
}
