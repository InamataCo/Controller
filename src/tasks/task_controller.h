#pragma once

#include <ArduinoJson.h>
#include <TaskSchedulerDeclarations.h>

#include "base_task.h"
#include "managers/mqtt.h"
#include "task_factory.h"

namespace bernd_box {
namespace tasks {

/**
 * This class is responsible for task creation, memory management and deletion.
 *
 * For usage instructions see the API.md documentation
 */
class TaskController {
 public:
  TaskController(Scheduler& scheduler, TaskFactory& factory, Mqtt& mqtt);
  virtual ~TaskController() = default;

  /**
   * Callback for incoming MQTT messages for the TaskFactory
   *
   * @param topic MQTT topic of the message
   * @param payload The message in bytes
   * @param length Length of the message in bytes
   */
  void mqttCallback(char* topic, uint8_t* payload, unsigned int length);

  /**
   * Create a new task
   *
   * @param parameters JSON object with the parameters to create a task
   * @return True on success
   */
  bool createTask(const JsonObjectConst& parameters);

  /**
   * Command a task to end
   *
   * @param parameters JSON object with the parameters to create a task
   * @return True on success
   */
  bool stopTask(const JsonObjectConst& parameters);

  /**
   * Sends the current status of the task factory
   *
   */
  void sendStatus(const JsonObjectConst& parameers);

  /// Name of the command to create a task (last subtopic of MQTT message)
  const __FlashStringHelper* mqtt_add_suffix_ = F("add");
  /// Name of the command to stop a task (last subtopic of MQTT message)
  const __FlashStringHelper* mqtt_remove_suffix_ = F("remove");
  /// Name of the command to return the status (last subtopic of MQTT message)
  const __FlashStringHelper* mqtt_status_suffix = F("status");
  /// Name of the command to return the status (last subtopic of MQTT message)
  const __FlashStringHelper* task_type_system_task = F("SystemTask");

 private:
  Task* findTask(unsigned int id);
  const __FlashStringHelper* getTaskType(Task* task);

  Scheduler& scheduler_;
  TaskFactory& factory_;
  Mqtt& mqtt_;
};
}  // namespace tasks
}  // namespace bernd_box