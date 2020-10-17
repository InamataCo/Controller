#pragma once

#include <ArduinoJson.h>
#include <TaskSchedulerDeclarations.h>

#include <map>
#include <memory>

#include "base_task.h"
#include "invalid_task.h"
#include "managers/server.h"

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
   * @param server Server object to send success and error notifications
   */
  TaskFactory(Server& server, Scheduler& scheduler);
  virtual ~TaskFactory() = default;

  static const String& type();

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

  /**
   * Return all registered factories by name
   * 
   * \return A vector with all names
   */
  const std::vector<String> getFactoryNames();

 private:
   
  /// Get the callback map of the sub-factories to create new task objects
  static std::map<String, Factory>& getFactories();

  static String invalidFactoryTypeError(const String& type);

  /// Reference to the Server interface
  Server& server_;
  /// Refernce to the Scheduler
  Scheduler& scheduler_;

  const __FlashStringHelper* type_key_ = F("type");
  const __FlashStringHelper* type_key_error_ = F("Missing property: type (string)");
};

}  // namespace tasks
}  // namespace bernd_box
