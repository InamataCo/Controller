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

  static PeripheryTaskRemovalTask ptrt(scheduler_);
  ptrt.add(*this);
}

void PeripheryTask::OnTaskDisable() {}

PeripheryTaskRemovalTask::PeripheryTaskRemovalTask(Scheduler& scheduler)
    : Task(&scheduler) {
  scheduler.addTask(*this);
}

void PeripheryTaskRemovalTask::add(PeripheryTask& pt) {
  tasks_.insert(&pt);
  setIterations(1);
  enableIfNot();
}

bool PeripheryTaskRemovalTask::Callback() {
  for (auto it = tasks_.begin(); it != tasks_.end(); ++it) {
    PeripheryTask* task = *it;
    delete task;
    tasks_.erase(it);
  }
  tasks_.clear();
  return true;
}

}  // namespace periphery
}  // namespace bernd_box
