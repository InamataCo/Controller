#include "abstract_periphery.h"

namespace bernd_box {
namespace periphery {

TaskFactory& AbstractPeriphery::getTaskFactory(const JsonObjectConst& doc) {
  JsonVariantConst task = doc[F("task")];
  if (task.isNull() || !task.is<char*>()) {
    return error_factory_;
  }

  auto taskFactoriesIterator = task_factories_.find(task);
  if (taskFactoriesIterator == task_factories_.end()) {
    return error_factory_;
  }

  return taskFactoriesIterator->second;
}

void AbstractPeriphery::addTaskFactory(const String& type,
                                       TaskFactory& taskFactory) {
  task_factories_.insert(std::pair<String, TaskFactory&>(type, taskFactory));
  avaiable_tasks_.push_back(type);
}

void AbstractPeriphery::addTaskFactory(
    std::map<String, TaskFactory&>& task_factories) {
  task_factories_.insert(task_factories.begin(), task_factories.end());
  for (auto it = task_factories.begin(); it != task_factories.end(); ++it) {
    avaiable_tasks_.push_back(it->first);
  }
}

const std::list<String>& AbstractPeriphery::getAvaiableTasks(){
  return avaiable_tasks_;
}

const bool AbstractPeriphery::isValid() { return isValid_; }

void AbstractPeriphery::setInvalid() { isValid_ = false; }

}  // namespace periphery
}  // namespace bernd_box
