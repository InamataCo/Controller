#include "base_task.h"

namespace bernd_box {
namespace tasks {

BaseTask::BaseTask(Scheduler& scheduler)
    : Task(&scheduler), scheduler_(scheduler) {}

bool BaseTask::OnEnable() {
  if (isValid()) {
    return OnTaskEnable();
  } else {
    return false;
  }
}

bool BaseTask::OnTaskEnable() { return true; }

void BaseTask::OnDisable() {
  OnTaskDisable();
  static TaskRemovalTask ptrt(scheduler_);
  ptrt.add(*this);
}

void BaseTask::OnTaskDisable() {}

bool BaseTask::isValid() const { return is_valid_; }

ErrorResult BaseTask::getError() const {
  if (is_valid_) {
    return ErrorResult();
  } else {
    return ErrorResult(getType(), error_message_);
  }
}

void BaseTask::setInvalid() { is_valid_ = false; }

void BaseTask::setInvalid(const String& error_message) {
  is_valid_ = false;
  error_message_ = error_message_;
}

String BaseTask::peripheralNotFoundError(const utils::UUID& uuid) {
  String error(peripheral_not_found_error_);
  error += uuid.toString();
  return error;
}

const __FlashStringHelper* BaseTask::peripheral_key_ = F("peripheral");
const __FlashStringHelper* BaseTask::peripheral_key_error_ =
    F("Missing property: peripheral (uuid)");
const __FlashStringHelper* BaseTask::peripheral_not_found_error_ =
    F("Could not find peripheral: ");

TaskRemovalTask::TaskRemovalTask(Scheduler& scheduler) : Task(&scheduler) {
  scheduler.addTask(*this);
}

void TaskRemovalTask::add(Task& pt) {
  tasks_.insert(&pt);
  setIterations(1);
  enableIfNot();
}

bool TaskRemovalTask::Callback() {
  for (auto it = tasks_.begin(); it != tasks_.end();) {
    Task* task = *it;

    BaseTask* base_task = dynamic_cast<BaseTask*>(task);
    if (base_task) {
      delete base_task;
      it = tasks_.erase(it);
    } else {
      Serial.println(F("Attempted to delete non-base class"));
      ++it;
    }
  }
  tasks_.clear();
  return true;
}

}  // namespace tasks

}  // namespace bernd_box
