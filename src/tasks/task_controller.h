#pragma once

#include <TaskSchedulerDeclarations.h>

#include <memory>

#include "base_task.h"
#include "managers/service_getters.h"
#include "task_factory.h"

namespace bernd_box {
namespace tasks {

/**
 * This class is responsible for task creation, memory management and deletion.
 *
 * For usage instructions see the API.md documentation
 */
class TaskController {
  friend class TaskRemovalTask;

 public:
  TaskController(Scheduler& scheduler, TaskFactory& factory);
  virtual ~TaskController() = default;

  static const String& type();

  void setServices(ServiceGetters services);

  /**
   * Callback for messages regarding tasks from the server
   *
   * @param message The message as a JSON doc
   */
  void handleCallback(const JsonObjectConst& message);

  /**
   * Gets all currently running task IDs
   *
   * \return A vector with the task IDs
   */
  std::vector<utils::UUID> getTaskIDs();

 private:
  /**
   * Start a new task
   *
   * @param parameters JSON object with the parameters to start a task
   * @return True on success
   */
  ErrorResult startTask(const ServiceGetters& services,
                        const JsonObjectConst& parameters);

  /**
   * Command a task to end
   *
   * @param parameters JSON object with the parameters to start a task
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
  static void addResultEntry(const utils::UUID& uuid, const ErrorResult& error,
                             const JsonArray& results);

  Scheduler& scheduler_;
  TaskFactory& factory_;
  ServiceGetters services_;

  static const __FlashStringHelper* task_command_key_;
  static const __FlashStringHelper* start_command_key_;
  static const __FlashStringHelper* stop_command_key_;
  static const __FlashStringHelper* status_command_key_;

  static const __FlashStringHelper* task_results_key_;
  static const __FlashStringHelper* result_status_key_;
  static const __FlashStringHelper* result_detail_key_;
  static const __FlashStringHelper* result_success_name_;
  static const __FlashStringHelper* result_fail_name_;

  static const String task_type_system_task_;
};
}  // namespace tasks
}  // namespace bernd_box