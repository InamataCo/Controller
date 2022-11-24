#pragma once

#include <set>

#include "managers/service_getters.h"
#include "tasks/base_task.h"

namespace inamata {
namespace tasks {

using namespace std::placeholders;

/**
 * Internal task to remove tasks that finished or are disabled
 *
 * When the task scheduler calls a task's OnDisable function, the task is added
 * to an instance of this task's removal queue. Once activated, it deletes all
 * queued tasks which thereby remove themselves.
 */
class TaskRemovalTask : public Task {
 public:
  TaskRemovalTask(Scheduler& scheduler);
  virtual ~TaskRemovalTask() = default;

  static const String& type();

  void setServices(ServiceGetters services);

  /**
   * Adds a task to the task removal queue
   *
   * \param to_be_removed The task to be deleted and removed
   */
  void add(Task& to_be_removed);

 private:
  /**
   * Goes through all tasks in the removal queue and deletes them
   *
   * \return Always true
   */
  bool Callback();

  /// Interface to send data to the server
  ServiceGetters services_;

  /// Queued tasks to be removed
  std::set<Task*> tasks_;
};

}  // namespace tasks
}  // namespace inamata