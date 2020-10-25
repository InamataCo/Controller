#pragma once

#include <ArduinoJson.h>
#include <TaskSchedulerDeclarations.h>

#include "base_task.h"
#include "managers/server.h"
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
  TaskController(Scheduler& scheduler, TaskFactory& factory, Server& server);
  virtual ~TaskController() = default;

  static const String& type();

  /**
   * Callback for messages regarding tasks from the server
   *
   * @param message The message as a JSON doc
   */
  void handleCallback(const JsonObjectConst& message);

 private:
  /**
   * Create a new task
   *
   * @param parameters JSON object with the parameters to create a task
   * @return True on success
   */
  ErrorResult createTask(const JsonObjectConst& parameters);

  /**
   * Command a task to end
   *
   * @param parameters JSON object with the parameters to create a task
   * @return True on success
   */
  ErrorResult stopTask(const JsonObjectConst& parameters);

  /**
   * Sends the current status of the task factory
   */
  void sendStatus();

  /**
   * Find the base task by UUID
   * 
   * \param uuid The ID of the base task
   * \return Pointer to the found base task
   */
  BaseTask* findTask(const utils::UUID& uuid);
  const String& getTaskType(Task* task);

  static void addResultEntry(const JsonVariantConst& uuid,
                             const ErrorResult& error,
                             const JsonArray& results);

  Scheduler& scheduler_;
  TaskFactory& factory_;
  Server& server_;

  static const __FlashStringHelper* task_command_key_;
  static const __FlashStringHelper* create_command_key_;
  static const __FlashStringHelper* stop_command_key_;
  static const __FlashStringHelper* status_command_key_;

  static const String task_type_system_task_;
};
}  // namespace tasks
}  // namespace bernd_box