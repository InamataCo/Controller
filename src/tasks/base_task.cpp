#include "base_task.h"

namespace bernd_box {
namespace tasks {

BaseTask::BaseTask(Scheduler& scheduler)
    : Task(&scheduler), scheduler_(scheduler) {}

void BaseTask::OnDisable() {
  OnTaskDisable();
  static TaskRemovalTask ptrt(scheduler_);
  ptrt.add(*this);
}

void BaseTask::OnTaskDisable() {}

bool BaseTask::isValid() { return is_valid_; }

void BaseTask::setInvalid() { is_valid_ = false; }

TaskRemovalTask::TaskRemovalTask(Scheduler& scheduler) : Task(&scheduler) {
  scheduler.addTask(*this);
}

void TaskRemovalTask::add(Task& pt) {
  tasks_.insert(&pt);
  setIterations(1);
  enableIfNot();
}

bool TaskRemovalTask::Callback() {
  for (auto it = tasks_.begin(); it != tasks_.end(); ++it) {
    Task* task = *it;
    delete task;
    tasks_.erase(it);
  }
  tasks_.clear();
  return true;
}

}  // namespace tasks

}  // namespace bernd_box
