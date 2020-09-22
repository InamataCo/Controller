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

  const String& type();

  /**
   * Callback for incoming MQTT messages for the TaskFactory
   *
   * @param message The message as a JSON doc
   */
  void handleCallback(const JsonObjectConst& message);

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
   *
   */
  void sendStatus();

  const __FlashStringHelper* task_command_name_ = F("task");
  const __FlashStringHelper* trace_id_name_ = F("id");
  const __FlashStringHelper* create_command_name_ = F("create");
  const __FlashStringHelper* stop_command_name_ = F("stop");
  const __FlashStringHelper* status_command_name_ = F("status");

  static const String task_type_system_task_;

 private:
  Task* findTask(unsigned int id);
  const String& getTaskType(Task* task);

  Scheduler& scheduler_;
  TaskFactory& factory_;
  Server& server_;
};
}  // namespace tasks
}  // namespace bernd_box