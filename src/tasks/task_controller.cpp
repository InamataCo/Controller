#include "task_controller.h"

namespace bernd_box {
namespace tasks {

TaskController::TaskController(Scheduler& scheduler, TaskFactory& factory,
                               Mqtt& mqtt)
    : scheduler_(scheduler), factory_(factory), mqtt_(mqtt){};

void TaskController::mqttCallback(char* topic, uint8_t* payload,
                                  unsigned int length) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  // Extract the last part of the topic (with a reverse search)
  char* command = strrchr(topic, '/');
  command++;
  String command_str(command);

  DynamicJsonDocument doc(BB_MQTT_JSON_PAYLOAD_SIZE);
  const DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    mqtt_.sendError(who, String(F("Deserialize failed: ")) + error.c_str());
    return;
  }

  if (command_str.equals(mqtt_add_suffix_)) {
    createTask(doc.as<JsonObjectConst>());
  } else if (command_str.equals(mqtt_remove_suffix_)) {
    stopTask(doc.as<JsonObjectConst>());
  } else if (command_str.equals(mqtt_status_suffix)) {
    sendStatus(doc.as<JsonObjectConst>());
  } else {
    mqtt_.sendError(
        who, String(F("Unknown action [add, remove, status]: ")) + command);
  }
}

bool TaskController::createTask(const JsonObjectConst& parameters) {
  BaseTask* task = factory_.createTask(parameters);
  if (task && task->isValid()) {
    task->enable();
    return true;
  }
  return false;
}

bool TaskController::stopTask(const JsonObjectConst& parameters) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  JsonVariantConst task_id = parameters[F("id")];
  if (!task_id.is<int>()) {
    mqtt_.sendError(who, "Missing property: id (int)");
    return false;
  }

  Task* task = findTask(task_id);
  if (task) {
    task->disable();
  } else {
    mqtt_.sendError(who, String(F("Could not find task with ID: ")) +
                             String(task_id.as<int>()));
  }
  return task != nullptr;
}

void TaskController::sendStatus(const JsonObjectConst& parameters) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);
  DynamicJsonDocument doc(BB_MQTT_JSON_PAYLOAD_SIZE);
  JsonObject status_object = doc.createNestedObject("status");
  JsonArray tasks_array = status_object.createNestedArray("tasks");

  for (Task* task = scheduler_.iFirst; task; task = task->iNext) {
    JsonObject task_object = tasks_array.createNestedObject();
    task_object["id"] = task->getId();
    task_object["type"] = getTaskType(task);
  }
  mqtt_.send(who, doc);
}

Task* TaskController::findTask(unsigned int id) {
  for (Task* task = scheduler_.iFirst; task; task = task->iNext) {
    if (task->getId() == id) {
      return task;
    }
  }
  return nullptr;
}

const __FlashStringHelper* TaskController::getTaskType(Task* task) {
  BaseTask* base_task = dynamic_cast<BaseTask*>(task);
  if (base_task) {
    return base_task->getType();
  } else {
    return task_type_system_task;
  }
}

}  // namespace tasks
}  // namespace bernd_box