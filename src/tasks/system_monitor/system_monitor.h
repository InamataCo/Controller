#pragma once

#include <TaskSchedulerDeclarations.h>

#include <chrono>

#include "managers/service_getters.h"
#include "tasks/base_task.h"

namespace inamata {
namespace tasks {
namespace system_monitor {

/**
 * Monitors the controller's state and informs the coordinator about it. Health
 * parameters include free memory and heap fragmentation.
 */
class SystemMonitor : public BaseTask {
 public:
  SystemMonitor(const ServiceGetters& services, Scheduler& scheduler);
  virtual ~SystemMonitor();

  const String& getType() const final;
  static const String& type();

  /**
   * Specifies how often the system monitor should check and send its state
   *
   * \param interval How often the check should be performed
   */
  void SetInterval(std::chrono::milliseconds interval);

 private:
  /**
   * Resets the CPU load counters and delays the task for one iteration
   *
   * \return true
   */
  bool OnTaskEnable() final;

  /**
   * Measure the state of the heap.
   *
   * \return true
   */
  bool TaskCallback() final;

  Scheduler& scheduler_;
  ServiceGetters services_;
  std::shared_ptr<Server> server_;

  // Max time is ~72 minutes due to an overflow in the CPU load counter
  static const std::chrono::seconds default_interval_;
};

}  // namespace system_monitor
}  // namespace tasks
}  // namespace inamata
