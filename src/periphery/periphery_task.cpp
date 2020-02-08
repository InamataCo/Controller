#include "periphery_task.h"

#include "managers/services.h"

namespace bernd_box {
namespace periphery {

PeripheryTask::PeripheryTask(std::shared_ptr<Periphery> periphery)
    : Task(&Services::getScheduler()),
      periphery_(periphery),
      scheduler_(Services::getScheduler()) {}

std::shared_ptr<Periphery> PeripheryTask::getPeriphery() { return periphery_; }

void PeripheryTask::OnDisable() {
  OnTaskDisable();

  static TaskRemovalTask ptrt(scheduler_);
  ptrt.add(*this);
}

void PeripheryTask::OnTaskDisable() {}

TaskRemovalTask::TaskRemovalTask(Scheduler& scheduler)
    : Task(&scheduler) {
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

}  // namespace periphery
}  // namespace bernd_box
