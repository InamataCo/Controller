#include "abstract_periphery.h"

namespace bernd_box {
namespace periphery {

std::unique_ptr<PeripheryTask> ErrorTaskFactory::createTask(
      std::shared_ptr<Periphery> periphery, const JsonObjectConst& doc){
        return nullptr;
      }

TaskFactory& AbstractPeriphery::getTaskFactory(const JsonObjectConst& doc) {
  JsonVariantConst task = doc[F("task")];
  if (task.isNull() || !task.is<char*>()) {
    return errorFactory_;
  }

  auto taskFactoriesIterator = taskFactories_.find(task);
  if (taskFactoriesIterator == taskFactories_.end()) {
    return errorFactory_;
  }

  return taskFactoriesIterator->second;
}

void AbstractPeriphery::addTaskFactory(const String& type,
                                       TaskFactory& taskFactory) {
  taskFactories_.insert(std::pair<String, TaskFactory&>(type, taskFactory));
}

void AbstractPeriphery::addTaskFactory(
    std::map<String, TaskFactory&>& taskFactories) {
  taskFactories_.insert(taskFactories.begin(), taskFactories.end());
}

const bool AbstractPeriphery::isValid() { return isValid_; }

void AbstractPeriphery::setInvalid() { isValid_ = false; }

}  // namespace periphery
}  // namespace bernd_box
