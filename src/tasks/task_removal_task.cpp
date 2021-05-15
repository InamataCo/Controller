#include "task_removal_task.h"

#include <ArduinoJson.h>
#include <TaskSchedulerDeclarations.h>

#include <functional>

#include "tasks/task_controller.h"

namespace bernd_box {
namespace tasks {

TaskRemovalTask::TaskRemovalTask(Scheduler& scheduler) : Task(&scheduler) {
  BaseTask::setTaskRemovalCallback(std::bind(&TaskRemovalTask::add, this, _1));
  // scheduler.addTask(*this); // TODO: Is this a bug? Superfluous addTask call?
}

const String& TaskRemovalTask::type() {
  static const String name{"TaskRemovalTask"};
  return name;
}

void TaskRemovalTask::setServices(const ServiceGetters& services) {
  server_ = services.get_server();
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
      TaskController::addResultEntry(base_task->getTaskID(),
                                     base_task->getError(), stop_results);
      delete base_task;
      it = tasks_.erase(it);
    } else {
      if (server_ != nullptr) {
        server_->sendError(
            ErrorResult(type(), F("Attempted to delete non-base class")));
      } else {
        Serial.println(
            ErrorResult(type(), ServiceGetters::server_nullptr_error_)
                .toString());
      }
      ++it;
    }
  }
  tasks_.clear();
  if (server_ != nullptr) {
    server_->sendResults(result_doc.as<JsonObject>());
  } else {
    Serial.println(
        ErrorResult(type(), ServiceGetters::server_nullptr_error_).toString());
  }

  return true;
}

}  // namespace tasks
}  // namespace bernd_box