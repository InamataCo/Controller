#include "invalid_task.h"

namespace bernd_box {
namespace tasks {

InvalidTask::InvalidTask(Scheduler& scheduler) : BaseTask(scheduler) {
  setInvalid();
}

InvalidTask::InvalidTask(Scheduler& scheduler, const String& error)
    : BaseTask(scheduler) {
  setInvalid(error);
}

void InvalidTask::TaskCallback() {}

const String& InvalidTask::getType() const { return type(); }

const String& InvalidTask::type() {
  static const String name{"InvalidTask"};
  return name;
}

}  // namespace tasks
}  // namespace bernd_box