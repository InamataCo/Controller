#include "task_controller.h"

namespace bernd_box {
namespace tasks {

TaskController::TaskController(Scheduler& scheduler, TaskFactory& factory,
                               Server& server)
    : scheduler_(scheduler), factory_(factory), server_(server){};

const String& TaskController::type() {
  static const String name{"TaskController"};
  return name;
}

void TaskController::handleCallback(const JsonObjectConst& message) {
  // Check if any task commands have to be processed
  JsonVariantConst task_commands = message[task_command_key_];
  if (!task_commands) {
    return;
  }

  // Init the result doc with type and the request ID
  DynamicJsonDocument result_doc(BB_JSON_PAYLOAD_SIZE);
  result_doc[Server::type_key_] = Server::type_result_name_;
  result_doc[Server::request_id_key_] = message[Server::request_id_key_];
  JsonObject task_results = result_doc.createNestedObject(task_command_key_);

  // Create a task for each command and store the result
  JsonArrayConst create_commands =
      task_commands[create_command_key_].as<JsonArrayConst>();
  if (create_commands) {
    JsonArray create_results =
        task_results.createNestedArray(create_command_key_);
    for (JsonVariantConst create_command : create_commands) {
      ErrorResult error = createTask(create_command);
      addResultEntry(create_command[BaseTask::uuid_key_], error,
                     create_results);
    }
  }

  // Remove a task for each command and store the result
  JsonArrayConst stop_commands =
      task_commands[stop_command_key_].as<JsonArrayConst>();
  if (stop_commands) {
    JsonArray stop_results = task_results.createNestedArray(stop_command_key_);
    for (JsonVariantConst stop_command : stop_commands) {
      ErrorResult error = stopTask(stop_command);
      addResultEntry(stop_command[BaseTask::uuid_key_], error, stop_results);
    }
  }

  // Send the status for each running task
  if (!task_commands[status_command_key_].isNull()) {
    sendStatus();
  }

  // Send the command results
  server_.sendResults(result_doc.as<JsonObject>());
}

ErrorResult TaskController::createTask(const JsonObjectConst& parameters) {
  BaseTask* task = factory_.createTask(parameters);
  if (task) {
    task->enable();
    return task->getError();
  } else {
    return ErrorResult(type(), "Unable to create task");
  }
}

ErrorResult TaskController::stopTask(const JsonObjectConst& parameters) {
  utils::UUID task_uuid(parameters[BaseTask::uuid_key_]);
  if (!task_uuid.isValid()) {
    return ErrorResult(type(), BaseTask::uuid_key_error_);
  }

  BaseTask* base_task = findTask(task_uuid);
  if (base_task) {
    base_task->disable();
  } else {
    return ErrorResult(type(), F("Could not find task"));
  }

  return ErrorResult();
}

void TaskController::sendStatus() {
  DynamicJsonDocument doc(BB_JSON_PAYLOAD_SIZE);
  JsonObject status_object = doc.createNestedObject("status");
  JsonArray tasks_array = status_object.createNestedArray("tasks");

  for (Task* task = scheduler_.iFirst; task; task = task->iNext) {
    BaseTask* base_task = dynamic_cast<BaseTask*>(task);
    if (base_task) {
      JsonObject task_object = tasks_array.createNestedObject();
      task_object["task"] = base_task->getUUID().toString();
      task_object["type"] = base_task->getType().c_str();
    }
  }
  server_.send(type(), doc);
}

BaseTask* TaskController::findTask(const utils::UUID& uuid) {
  // Go through all tasks in the scheduler
  for (Task* task = scheduler_.iFirst; task; task = task->iNext) {
    // Check if it is a base task
    BaseTask* base_task = dynamic_cast<BaseTask*>(task);
    // If the UUIDs match, return the task and end the search
    if (base_task && base_task->getUUID() == uuid) {
      return base_task;
    }
  }

  return nullptr;
}

const String& TaskController::getTaskType(Task* task) {
  BaseTask* base_task = dynamic_cast<BaseTask*>(task);
  if (base_task) {
    return base_task->getType();
  } else {
    return task_type_system_task_;
  }
}

void TaskController::addResultEntry(const JsonVariantConst& uuid,
                                    const ErrorResult& error,
                                    const JsonArray& results) {
  JsonObject result = results.createNestedObject();

  // Save whether the peripheral could be created or the reason for failing
  if (error.isError()) {
    result[BaseTask::uuid_key_] = uuid;
    result["status"] = "fail";
    result["detail"] = error.detail_;
  } else {
    result[BaseTask::uuid_key_] = uuid;
    result["status"] = "success";
  }
}

const __FlashStringHelper* TaskController::task_command_key_ = F("task");
const __FlashStringHelper* TaskController::create_command_key_ = F("create");
const __FlashStringHelper* TaskController::stop_command_key_ = F("stop");
const __FlashStringHelper* TaskController::status_command_key_ = F("status");

const String TaskController::task_type_system_task_{"SystemTask"};

}  // namespace tasks
}  // namespace bernd_box