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
class TaskFactory {
 public:
  /// Callback to create a task
  using Factory = BaseTask* (*)(
      const JsonObjectConst& parameters, Scheduler& scheduler);

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
   * Create a Task object from a JSON object by passing it to the subfactories
   *
   * @param parameters JSON object with the parameters to create a task
   * @return True on success
   */
  BaseTask* createTask(const JsonObjectConst& parameters);

 private:

  /// Callback map of the sub-factories to create new task objects
  static std::map<String, Factory> factories_;

  /// Reference to the MQTT interface
  Mqtt& mqtt_;
  /// Refernce to the Scheduler
  Scheduler& scheduler_;
};

}  // namespace tasks
}  // namespace bernd_box
