#pragma once

#include "base_task.h"

namespace bernd_box {
namespace tasks {

class InvalidTask : public BaseTask {
 public:
  /**
   * Create a task with an invalid state
   * 
   * \param scheduler The TaskScheduler to attach the task to
   */
  InvalidTask(Scheduler& scheduler);

  /**
   * Create an task with an invalid state and error reason
   * 
   * \param scheduler The TaskScheduler to attach the task to
   * \param error The reason for being invalid
   */
  InvalidTask(Scheduler& scheduler, const String& error);
  virtual ~InvalidTask() = default;

  /**
   * Never runs as set invalid in constructor
   */
  bool TaskCallback() final;

  const String& getType() const;
  static const String& type();
};

}  // namespace tasks
}  // namespace bernd_box
