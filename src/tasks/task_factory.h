#pragma once

#include <ArduinoJson.h>
#include <TaskSchedulerDeclarations.h>

#include <map>
#include <memory>

#include "base_task.h"
#include "managers/mqtt.h"

namespace bernd_box {
namespace tasks {

/**
 * This class is responsible for task creation, memory management and deletion.
 *
 * For usage instructions see the API.md documentation
 */
class TaskFactory : public BaseTask {
 public:
  /// Callback to create a task
  using Factory = std::unique_ptr<BaseTask> (*)(
      const JsonObjectConst& parameters, Scheduler& scheduler,
      BaseTask::RemoveCallback remove_callback);

  /**
   * Create a task factory that forwards 'add' commands to the subfactories
   *
   * In order to delete tasks after they have ended, the task factory acts as
   * a task itself to delete the task object after it has been disabled.
   *
   * @param mqtt MQTT object to send success and error notifications
   */
  TaskFactory(Mqtt& mqtt, Scheduler& scheduler);
  virtual ~TaskFactory() = default;

  /**
   * Register a task factory as a callback to create a task
   *
   * @param type Name of the task factory
   * @param factory Callback to the task factory
   * @return True on success --> no type by that name exists
   */
  static bool registerTask(const String& type, Factory factory);

  /**
   * Get the object's type and used in polymorphic situations
   * 
   * @return Name of the type
   */
  const __FlashStringHelper* getType() final;

  /**
   * Get the object's type and used during the static initialization
   * 
   * @return Name of the type
   */
  static const __FlashStringHelper* type();

  /**
   * Callback for incoming MQTT messages for the TaskFactory
   *
   * @param topic MQTT topic of the message
   * @param payload The message in bytes
   * @param length Length of the message in bytes
   */
  void mqttCallback(char* topic, uint8_t* payload, unsigned int length);

  /**
   * Create a Task object from a JSON object by passing it to the subfactories
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

 private:
  /**
   * Returns the ID for the newly created task
   *
   * @return ID of the new task
   */
  const int getNextTaskId();

  /**
   * Remove a task after it has been stopped
   *
   * @param id ID of the task to be stopped
   * @return True on success
   */
  void markTaskForRemoval(const int id);

  /**
   * Function to handle removal of stopped tasks
   *
   * @return True on success
   */
  bool Callback();

  /// Name of the command to create a task (last subtopic of MQTT message)
  const __FlashStringHelper* mqtt_add_suffix_ = F("add");
  /// Name of the command to stop a task (last subtopic of MQTT message)
  const __FlashStringHelper* mqtt_remove_suffix_ = F("remove");
  /// Name of the command to return the status (last subtopic of MQTT message)
  const __FlashStringHelper* mqtt_status_suffix = F("status");

  /// Callback map of the sub-factories to create new task objects
  static std::map<String, Factory> factories_;
  /// Manages lifetime of tasks
  std::map<int, std::unique_ptr<BaseTask>> tasks_;
  /// Tasks to be removed in the next scheduler pass
  std::vector<int> removal_task_ids_;

  /// Reference to the MQTT interface
  Mqtt& mqtt_;
  /// Refernce to the Scheduler
  Scheduler& scheduler_;

  /// ID of the next task to be created
  int next_task_id_;
  /// Its own task ID to prevent it from being removed
  int task_id_ = next_task_id_++;
};

}  // namespace tasks
}  // namespace bernd_box
