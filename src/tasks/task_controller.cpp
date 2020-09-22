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
  JsonVariantConst task_commands = message[task_command_name_];

  // Handle all create commands
  for (JsonVariantConst create_command :
       task_commands[create_command_name_].as<JsonArrayConst>()) {
    ErrorResult error = createTask(create_command);
    if (error.is_error()) {
      server_.sendError(error, message[trace_id_name_].as<String>());
    }
  }

  for (JsonVariantConst stop_command :
       task_commands[stop_command_name_].as<JsonArrayConst>()) {
    ErrorResult error = createTask(stop_command);
    if (error.is_error()) {
      server_.sendError(error, message[trace_id_name_].as<String>());
    }
  }

  if (!task_commands[status_command_name_].isNull()) {
    sendStatus();
  }
}

ErrorResult TaskController::createTask(const JsonObjectConst& parameters) {
  BaseTask* task = factory_.createTask(parameters);
  if (task && task->isValid()) {
    task->enable();
    return ErrorResult();
  }

  return ErrorResult(type(), "Unable to create task");
}

ErrorResult TaskController::stopTask(const JsonObjectConst& parameters) {
  JsonVariantConst task_id = parameters[F("id")];
  if (!task_id.is<int>()) {
    return ErrorResult(type(), "Missing property: id (int)");
  }

  Task* task = findTask(task_id);
  if (task) {
    task->disable();
  } else {
    return ErrorResult(
        type(), String(F("Could not find task with ID")) + String(task_id.as<int>()));
  }

  return ErrorResult();
}

void TaskController::sendStatus() {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);
  DynamicJsonDocument doc(BB_JSON_PAYLOAD_SIZE);
  JsonObject status_object = doc.createNestedObject("status");
  JsonArray tasks_array = status_object.createNestedArray("tasks");

  for (Task* task = scheduler_.iFirst; task; task = task->iNext) {
    JsonObject task_object = tasks_array.createNestedObject();
    task_object["id"] = task->getId();
    task_object["type"] = getTaskType(task).c_str();
  }
  server_.send(who, doc);
}

Task* TaskController::findTask(unsigned int id) {
  for (Task* task = scheduler_.iFirst; task; task = task->iNext) {
    if (task->getId() == id) {
      return task;
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

const String TaskController::task_type_system_task_{"SystemTask"};

}  // namespace tasks
}  // namespace bernd_box