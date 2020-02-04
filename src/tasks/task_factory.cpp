#include "task_factory.h"

namespace bernd_box {
namespace tasks {

TaskFactory::TaskFactory(Mqtt& mqtt, Scheduler& scheduler)
    : ::Task(&scheduler), mqtt_(mqtt), scheduler_(scheduler) {}

bool TaskFactory::registerTask(const String& type, Factory factory) {
  return factories_.insert({type, factory}).second;
}

void TaskFactory::mqttCallback(char* topic, uint8_t* payload,
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

  if (command_str.equals(task_add_suffix_)) {
    createTask(doc.as<JsonObjectConst>());
    return;
  }

  if (command_str.equals(task_remove_suffix_)) {
    stopTask(doc.as<JsonObjectConst>());
    return;
  }

  mqtt_.sendError(who, String(F("Unknown action [add, remove]: ")) + command);
  return;
}

bool TaskFactory::createTask(const JsonObjectConst& parameters) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  JsonVariantConst type = parameters[F("type")];
  if (type.isNull() || !type.is<char*>()) {
    mqtt_.sendError(who, "Missing property: type (string)");
    return false;
  }

  // Check if a factory for the type exists. Then try to create such a task
  const auto& factory = factories_.find(type);
  if (factory != factories_.end()) {
    // Create a task via the respective task factory
    auto remover = [this](const int id) { return markTaskForRemoval(id); };
    auto task = factory->second(parameters, scheduler_, remover);

    // Expect the factory to return a nullptr on failure
    if (task) {
      const int task_id = getNextTaskId();
      tasks_.emplace(task_id, std::move(task));
      return true;
    } else {
      // Error reporting handled by sub-factory
      return false;
    }
  }

  mqtt_.sendError(
      who, String(F("Could not find the factory type: ")) + type.as<String>());
  return false;
}

bool TaskFactory::stopTask(const JsonObjectConst& parameters) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  JsonVariantConst task_id = parameters[F("id")];
  if (!task_id.is<int>()) {
    mqtt_.sendError(who, "Missing property: id (int)");
    return false;
  }

  const auto& task = tasks_.find(task_id);
  if (task != tasks_.end()) {
    task->second->disable();
    return true;
  } else {
    mqtt_.sendError(who, String(F("Could not find task with ID: ")) +
                             String(task_id.as<int>()));
    return false;
  }
}

const int TaskFactory::getNextTaskId() { return next_task_id_++; }

void TaskFactory::markTaskForRemoval(const int id) {
  // Add the task ID to the list to be removed and then enable the TaskFactory's
  // task to remove all tasks in the list
  removal_task_ids_.push_back(id);
  enableIfNot();
}

bool TaskFactory::Callback() {
  for (const auto task_id : removal_task_ids_) {
    const int tasks_erased = tasks_.erase(task_id);
    Serial.printf("%d tasks erased for task ID: %d", tasks_erased, task_id);
  }
  return false;
}

std::map<String, TaskFactory::Factory> TaskFactory::factories_;

}  // namespace tasks
}  // namespace bernd_box
