#include "abstractPeriphery.h"

namespace bernd_box {
namespace periphery {

AbstractPeriphery::AbstractPeriphery(Library& library, const String name)
    : name_(name), library_(library) {}

Result AbstractPeriphery::executeTask(const JsonObjectConst& doc) {
  JsonVariantConst task = doc[F("task")];
  if (task.isNull() || !task.is<char*>()) {
    return Result::kFailure;
  }
  JsonVariantConst parameter = doc[F("parameter")];
  if (parameter.isNull()) {
    return Result::kFailure;
  }

  std::map<String, TaskFactory&>::iterator iterator = taskFactories_.find(task);
  if (iterator == taskFactories_.end()) return Result::kFailure;

  TaskFactory& taskFactory = iterator->second;
  PeripheryTask& peripheryTask = taskFactory.createTask(parameter);
  Result result = peripheryTask.execute(*this);
  delete &peripheryTask;

  return result;
}

void AbstractPeriphery::addTaskFactory(const String& type,
                                       TaskFactory& taskFactory) {
  taskFactories_.insert(std::pair<String, TaskFactory&>(type, taskFactory));
}

void AbstractPeriphery::addTaskFactory(
    std::map<String, TaskFactory&>& taskFactories) {
  taskFactories_.insert(taskFactories.begin(), taskFactories.end());
}

Library& AbstractPeriphery::getLibrary() { return library_; }

}  // namespace periphery
}  // namespace bernd_box