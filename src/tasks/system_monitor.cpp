#include "system_monitor.h"

namespace bernd_box {
namespace tasks {

SystemMonitor::SystemMonitor(Scheduler* scheduler, Mqtt& mqtt)
    : Task(scheduler), mqtt_(mqtt), name_(F("system_monitor")) {
  setIterations(TASK_FOREVER);
  std::chrono::minutes default_interval(5);
  Task::setInterval(std::chrono::milliseconds(default_interval).count());
}
SystemMonitor::~SystemMonitor() {}

void SystemMonitor::setInterval(std::chrono::milliseconds interval) {
  Task::setInterval(interval.count());
}

bool SystemMonitor::Callback() {
  // Get the total available memory and largest continguous memory block.
  // This allows us to calculate the fragmentation index =
  //     (total free - largest free block) / total free * 100
  size_t free_bytes = heap_caps_get_free_size(MALLOC_CAP_8BIT);
  size_t max_malloc_bytes = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  size_t least_free_bytes = heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);

  DynamicJsonDocument doc(BB_MQTT_JSON_PAYLOAD_SIZE);
  doc["free_memory_bytes"] = free_bytes;
  doc["heap_fragmentation_percent"] =
      (float(free_bytes) - float(max_malloc_bytes)) / float(free_bytes) *
      float(100);
  doc["least_free_bytes"] = least_free_bytes;
  mqtt_.send(String(name_), doc);

  return true;
}

}  // namespace tasks
}  // namespace bernd_box
