#include "system_monitor.h"

namespace inamata {
namespace tasks {
namespace system_monitor {

SystemMonitor::SystemMonitor(const ServiceGetters& services,
                             Scheduler& scheduler)
    : BaseTask(scheduler), scheduler_(scheduler), services_(services) {
  setIterations(TASK_FOREVER);
  Task::setInterval(std::chrono::milliseconds(default_interval_).count());
}

SystemMonitor::~SystemMonitor() {}

const String& SystemMonitor::getType() const { return type(); }

const String& SystemMonitor::type() {
  static const String name{"SystemMonitor"};
  return name;
}

void SystemMonitor::SetInterval(std::chrono::milliseconds interval) {
  Task::setInterval(interval.count());
}

bool SystemMonitor::OnTaskEnable() {
  web_socket_ = services_.getWebSocket();
  if (web_socket_ == nullptr) {
    setInvalid(services_.network_nullptr_error_);
    return false;
  }
  // Reset counters to calculate CPU load. Wait one interval for valid readings
  scheduler_.cpuLoadReset();
  delay();

  return true;
}

bool SystemMonitor::TaskCallback() {
  // Get the total available memory and largest continguous memory block.
  // This allows us to calculate the fragmentation index =
  //     (total free - largest free block) / total free * 100
  size_t stack_hwm = 0;
  size_t free_bytes = 0;
  size_t max_malloc_bytes = 0;
  size_t least_free_bytes = 0;
  uint8_t heap_fragmentation = 0;
#ifdef ESP32
  stack_hwm = uxTaskGetStackHighWaterMark(NULL) * 4;
  free_bytes = heap_caps_get_free_size(MALLOC_CAP_8BIT);
  max_malloc_bytes = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  least_free_bytes = heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
#else
  stack_hwm = ESP.getFreeContStack();
  uint32_t heap_free;
  uint16_t heap_max;
  uint8_t heap_frag;
  ESP.getHeapStats(&heap_free, &heap_max, &heap_frag);
  free_bytes = heap_free;
  max_malloc_bytes = heap_max;
  heap_fragmentation = heap_frag;
#endif

  doc_out.clear();
  if (stack_hwm) {
    doc_out[F("stack_hwm_bytes")] = stack_hwm;
  }
  if (free_bytes) {
    doc_out[F("free_memory_bytes")] = free_bytes;
  }
  if (!heap_fragmentation && free_bytes && max_malloc_bytes) {
    heap_fragmentation = (float(free_bytes) - float(max_malloc_bytes)) /
                         float(free_bytes) * 100.0f;
  }
  if (heap_fragmentation) {
    doc_out[F("heap_fragmentation_percent")] = heap_fragmentation;
  }
  // Only on ESP32
  if (least_free_bytes) {
    doc_out[F("least_free_bytes")] = least_free_bytes;
  }

  float cpuTotal = scheduler_.getCpuLoadTotal();
  float cpuCycles = scheduler_.getCpuLoadCycle();
  float cpuIdle = scheduler_.getCpuLoadIdle();
  scheduler_.cpuLoadReset();

  // Productive work (not idle, not scheduling) --> time in task callbacks
  doc_out[F("productive_percent")] =
      100 - ((cpuIdle + cpuCycles) / cpuTotal * 100.0);
  doc_out[F("wifi_rssi")] = WiFi.RSSI();

  web_socket_->sendSystem(doc_out.as<JsonObject>());
  return true;
}

const std::chrono::seconds SystemMonitor::default_interval_{30};

}  // namespace system_monitor
}  // namespace tasks
}  // namespace inamata
