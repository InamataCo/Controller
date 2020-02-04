#include "base_task.h"

namespace bernd_box {
namespace tasks {

BaseTask::BaseTask(Scheduler& scheduler, RemoveCallback remove_callback)
    : Task(&scheduler) {}

void BaseTask::OnDisable() { OnTaskDisable(); }

void BaseTask::OnTaskDisable() {}

bool BaseTask::isValid() { return is_valid_; }

void BaseTask::setInvalid() { is_valid_ = false; }

}  // namespace tasks

}  // namespace bernd_box
