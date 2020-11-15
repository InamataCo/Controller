#include "task_removal_task.h"

namespace bernd_box {
namespace tasks {

TaskRemovalTask::TaskRemovalTask(Scheduler& scheduler, Server& server)
    : Task(&scheduler), server_(server) {
  BaseTask::setTaskRemovalCallback(std::bind(&TaskRemovalTask::add, this, _1));
  scheduler.addTask(*this);
}

const String& TaskRemovalTask::type() {
  static const String name{"TaskRemovalTask"};
  return name;
}

void TaskRemovalTask::add(Task& pt) {
  tasks_.insert(&pt);
  setIterations(1);
  enableIfNot();
}

bool TaskRemovalTask::Callback() {
  if (tasks_.empty()) {
    return true;
  }

  DynamicJsonDocument result_doc(BB_JSON_PAYLOAD_SIZE);
  result_doc[Server::type_key_] = Server::result_type_;
  JsonObject task_results =
      result_doc.createNestedObject(TaskController::task_results_key_);
  JsonArray stop_results =
      task_results.createNestedArray(TaskController::stop_command_key_);

  for (auto it = tasks_.begin(); it != tasks_.end();) {
    Task* task = *it;

    BaseTask* base_task = dynamic_cast<BaseTask*>(task);
    if (base_task) {
      TaskController::addResultEntry(base_task->getTaskID(), ErrorResult(),
                                     stop_results);
      delete base_task;
      it = tasks_.erase(it);
    } else {
      server_.sendError(
          ErrorResult(type(), F("Attempted to delete non-base class")));
      ++it;
    }
  }
  tasks_.clear();
  server_.sendResults(result_doc.as<JsonObject>());

  return true;
}

}  // namespace tasks
}  // namespace bernd_box