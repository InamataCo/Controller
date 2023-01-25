#include "task_removal_task.h"

#include <ArduinoJson.h>
#include <TaskSchedulerDeclarations.h>

#include <functional>

#include "tasks/task_controller.h"

namespace inamata {
namespace tasks {

TaskRemovalTask::TaskRemovalTask(Scheduler& scheduler) : Task(&scheduler) {
  BaseTask::setTaskRemovalCallback(std::bind(&TaskRemovalTask::add, this, _1));
  // scheduler.addTask(*this); // TODO: Is this a bug? Superfluous addTask call?
}

const String& TaskRemovalTask::type() {
  static const String name{"TaskRemovalTask"};
  return name;
}

void TaskRemovalTask::setServices(ServiceGetters services) {
  services_ = services;
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

  doc_out.clear();
  doc_out[WebSocket::type_key_] = WebSocket::result_type_;
  JsonObject task_results =
      doc_out.createNestedObject(TaskController::task_results_key_);
  JsonArray stop_results =
      task_results.createNestedArray(TaskController::stop_command_key_);

  for (auto it = tasks_.begin(); it != tasks_.end();) {
    Task* task = *it;
    BaseTask* base_task = dynamic_cast<BaseTask*>(task);
    TRACEF("Deleting: %s\n", base_task->getType());

    // If it is not a system task, delete the task and free the memory
    // System tasks have a static memory lifetime and should not be deleted
    if (base_task && !base_task->isSystemTask()) {
      TaskController::addResultEntry(base_task->getTaskID(),
                                     base_task->getError(), stop_results);
      delete base_task;
    }
    it = tasks_.erase(it);
  }
  tasks_.clear();
  if (stop_results.size()) {
    std::shared_ptr<WebSocket> web_socket = services_.getWebSocket();
    if (web_socket != nullptr) {
      web_socket->sendResults(doc_out.as<JsonObject>());
    } else {
      TRACELN(ErrorResult(type(), ServiceGetters::web_socket_nullptr_error_)
                         .toString());
    }
  }

  return true;
}

}  // namespace tasks
}  // namespace inamata