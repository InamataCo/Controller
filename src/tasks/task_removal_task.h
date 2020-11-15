#pragma once

#include <functional>
#include <set>

#include "tasks/base_task.h"
#include "tasks/task_controller.h"
#include "managers/server.h"

namespace bernd_box {
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
  TaskRemovalTask(Scheduler& scheduler, Server& server);
  virtual ~TaskRemovalTask() = default;

  static const String& type();

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

  /// Queued tasks to be removed
  std::set<Task*> tasks_;
  /// Server to send messages to
  Server& server_;
};

}  // namespace tasks
}  // namespace bernd_box