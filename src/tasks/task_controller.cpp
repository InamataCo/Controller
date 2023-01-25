#include "task_controller.h"

#include <ArduinoJson.h>

namespace inamata {
namespace tasks {

TaskController::TaskController(Scheduler& scheduler, TaskFactory& factory)
    : scheduler_(scheduler), factory_(factory){};

const String& TaskController::type() {
  static const String name{"TaskController"};
  return name;
}

void TaskController::setServices(ServiceGetters services) {
  services_ = services;
}

void TaskController::handleCallback(const JsonObjectConst& message) {
  TRACELN(F("Handling task cmd"));
  // Check if any task commands have to be processed
  JsonVariantConst task_commands = message[task_command_key_];
  if (!task_commands) {
    return;
  }

  // Init the result doc with type and the request ID
  doc_out.clear();
  doc_out[WebSocket::type_key_] = WebSocket::result_type_;
  JsonVariantConst request_id = message[WebSocket::request_id_key_];
  if (request_id) {
    doc_out[WebSocket::request_id_key_] = request_id;
  }
  JsonObject task_results = doc_out.createNestedObject(task_results_key_);

  // Start a task for each command and store the result
  JsonArrayConst start_commands =
      task_commands[start_command_key_].as<JsonArrayConst>();
  if (start_commands) {
    JsonArray start_results =
        task_results.createNestedArray(start_command_key_);
    for (JsonVariantConst start_command : start_commands) {
      ErrorResult error = startTask(services_, start_command);
      addResultEntry(start_command[BaseTask::task_id_key_], error,
                     start_results);
    }
  }

  // Remove a task for each command and store the result
  JsonArrayConst stop_commands =
      task_commands[stop_command_key_].as<JsonArrayConst>();
  if (stop_commands) {
    JsonArray stop_results = task_results.createNestedArray(stop_command_key_);
    for (JsonVariantConst stop_command : stop_commands) {
      ErrorResult error = stopTask(stop_command);
      // Tasks are only fully removed by the TaskRemovalTask. It will send a
      // stop success result when it succeeds, so only report errors
      if (error.isError()) {
        addResultEntry(stop_command[BaseTask::task_id_key_], error,
                       stop_results);
      }
    }
  }

  // Send the status for each running task
  if (!task_commands[status_command_key_].isNull()) {
    sendStatus();
  }

  // Send the command results
  std::shared_ptr<WebSocket> web_socket = services_.getWebSocket();
  if (web_socket != nullptr) {
    web_socket->sendResults(doc_out.as<JsonObject>());
  } else {
    TRACELN(
        ErrorResult(type(), services_.web_socket_nullptr_error_).toString());
  }
}

std::vector<utils::UUID> TaskController::getTaskIDs() {
  std::vector<utils::UUID> task_ids;

  for (Task* task = scheduler_.getFirstTask(); task != NULL;
       task = task->getNextTask()) {
    BaseTask* base_task = dynamic_cast<BaseTask*>(task);
    if (base_task) {
      task_ids.push_back(base_task->getTaskID());
    }
  }

  return task_ids;
}

ErrorResult TaskController::startTask(const ServiceGetters& services,
                                      const JsonObjectConst& parameters) {
  // Create a task and check if a nullptr was returned
  BaseTask* task = factory_.startTask(services, parameters);
  if (task) {
    // Try enabling the task and check if it could be enabled
    task->enable();
    ErrorResult error = task->getError();
    // On error, directly delete task without using the TaskRemovalTask to
    // avoid sending double stop notifications to the server.
    if (error.isError()) {
      delete task;
    }
    return error;
  } else {
    return ErrorResult(type(), "Unable to start task");
  }
}

ErrorResult TaskController::stopTask(const JsonObjectConst& parameters) {
  utils::UUID task_uuid(parameters[BaseTask::task_id_key_]);
  if (!task_uuid.isValid()) {
    return ErrorResult(type(), BaseTask::task_id_key_error_);
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
  doc_out.clear();
  JsonObject status_object = doc_out.createNestedObject("status");
  JsonArray tasks_array = status_object.createNestedArray("tasks");

  for (Task* task = scheduler_.iFirst; task; task = task->iNext) {
    BaseTask* base_task = dynamic_cast<BaseTask*>(task);
    if (base_task) {
      JsonObject task_object = tasks_array.createNestedObject();
      task_object["task"] = base_task->getTaskID().toString();
      task_object["type"] = base_task->getType().c_str();
    }
  }
  std::shared_ptr<WebSocket> web_socket = services_.getWebSocket();
  if (web_socket != nullptr) {
    web_socket->send(type(), doc_out);
  } else {
    TRACELN(
        ErrorResult(type(), services_.web_socket_nullptr_error_).toString());
  }
}

BaseTask* TaskController::findTask(const utils::UUID& uuid) {
  // Go through all tasks in the scheduler
  for (Task* task = scheduler_.iFirst; task; task = task->iNext) {
    // Check if it is a base task
    BaseTask* base_task = dynamic_cast<BaseTask*>(task);
    // If the UUIDs match, return the task and end the search
    if (base_task && base_task->getTaskID() == uuid) {
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

  // Save whether the task could be started or the reason for failing
  if (error.isError()) {
    result[BaseTask::task_id_key_] = uuid;
    result[result_status_key_] = result_fail_name_;
    result[result_detail_key_] = error.detail_;
  } else {
    result[BaseTask::task_id_key_] = uuid;
    result[result_status_key_] = result_success_name_;
  }
}

void TaskController::addResultEntry(const utils::UUID& uuid,
                                    const ErrorResult& error,
                                    const JsonArray& results) {
  JsonObject result = results.createNestedObject();

  // Save whether the task could be started or the reason for failing
  if (error.isError()) {
    result[BaseTask::task_id_key_] = uuid.toString();
    result[result_status_key_] = result_fail_name_;
    result[result_detail_key_] = error.detail_;
  } else {
    result[BaseTask::task_id_key_] = uuid.toString();
    result[result_status_key_] = result_success_name_;
  }
}

const __FlashStringHelper* TaskController::task_command_key_ = FPSTR("task");
const __FlashStringHelper* TaskController::start_command_key_ = FPSTR("start");
const __FlashStringHelper* TaskController::stop_command_key_ = FPSTR("stop");
const __FlashStringHelper* TaskController::status_command_key_ = FPSTR("status");

const __FlashStringHelper* TaskController::task_results_key_ = FPSTR("task");
const __FlashStringHelper* TaskController::result_status_key_ = FPSTR("status");
const __FlashStringHelper* TaskController::result_detail_key_ = FPSTR("detail");
const __FlashStringHelper* TaskController::result_success_name_ = FPSTR("success");
const __FlashStringHelper* TaskController::result_fail_name_ = FPSTR("fail");

const String TaskController::task_type_system_task_{"SystemTask"};

}  // namespace tasks
}  // namespace inamata