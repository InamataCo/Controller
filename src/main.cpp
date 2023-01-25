#include <Arduino.h>
#include <TaskScheduler.h>

#include "managers/services.h"
#include "utils/setup_node.h"

//----------------------------------------------------------------------------
// Global instances
inamata::Services services;
Scheduler& scheduler = services.getScheduler();

//----------------------------------------------------------------------------
// Setup and loop functions
void setup() {
  bool success = inamata::setupNode(services);
  if (success) {
    TRACELN(F("Setup finished"));
  } else {
    TRACELN(F("Node setup failed. Restarting in 10s"));
    delay(10000);
    ESP.restart();
  }
}

#ifdef MONITOR_MEMORY
std::chrono::steady_clock::time_point last_check;
#endif

void loop() {
  #ifdef MONITOR_MEMORY
  if (std::chrono::steady_clock::now() - last_check >
      std::chrono::milliseconds(500)) {
    last_check = std::chrono::steady_clock::now();
#ifdef ESP32
    Serial.printf("stack hwm: %u, free: %u, max_block: %u, least_free: %u\n",
                  uxTaskGetStackHighWaterMark(NULL) * 4,
                  heap_caps_get_free_size(MALLOC_CAP_8BIT),
                  heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
                  heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT)
    );
#else
    Serial.printf("stack %u, heap: %u, max_block: %u\n", ESP.getFreeContStack(),
                  ESP.getFreeHeap(), ESP.getMaxFreeBlockSize());
#endif
  }
  #endif
  scheduler.execute();
}
