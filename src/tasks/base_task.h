#pragma once

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>

#include <functional>
#include <set>

#include "managers/io_types.h"
#include "utils/uuid.h"

namespace bernd_box {
namespace tasks {

class BaseTask : public Task {
 public:
  /**
   * Constructor used by tasks created locally
   *
   * \param scheduler The scheduler that executes the tasks
   * \param uuid Unique identifier created locally
   */
  BaseTask(Scheduler& scheduler, utils::UUID task_id = utils::UUID());

  /**
   * Constructor used when commanded by the server
   *
   * \param scheduler The scheduler that executes the tasks
   * \param parameters The JSON doc should contain the key with the task's UUID
   */
  BaseTask(Scheduler& scheduler, const JsonObjectConst& parameters);

  virtual ~BaseTask() = default;

  virtual const String& getType() const = 0;

  /**
   * Called by the task scheduler and checks if the task is valid
   *
   * \return True if the task is valid
   */
  bool OnEnable() final;

  /**
   * For derived classes to perform init tasks and called when task is enabled
   *
   * \return True if the derived task inited successfully
   */
  virtual bool OnTaskEnable();

  /**
   * Called by the task scheduler and triggers task removal
   *
   * Contains a static TaskRemovalTask which is tasked with removing disabled
   * tasks.
   */
  void OnDisable() final;

  /**
   * For derived classes to perform clean up
   */
  virtual void OnTaskDisable();

  /**
   * Check if the task is in a valid state
   *
   * \return True if it is in a valid state
   */
  bool isValid() const;

  /**
   * Gets the task's error state, which gives the who and detail information
   *
   * \return The task's error state
   */
  ErrorResult getError() const;

  /**
   * Gets the task's UUID, which was created locally or on the server
   *
   * \return The task's UUID
   */
  const utils::UUID& getTaskID() const;

  static const __FlashStringHelper* peripheral_key_;
  static const __FlashStringHelper* peripheral_key_error_;
  static const __FlashStringHelper* peripheral_not_found_error_;
  static const __FlashStringHelper* task_id_key_;
  static const __FlashStringHelper* task_id_key_error_;

 protected:
  /**
   * Mark the task as being in an invalid state
   *
   * \see isValid()
   */
  void setInvalid();

  /**
   * Mark the task as being invalid and the cause
   *
   * \see isValid(), getError()
   *
   * \param error_message The cause for being invalid
   */
  void setInvalid(const String& error_message);

  static String peripheralNotFoundError(const utils::UUID& uuid);

 private:
  /// Whether the task is in a valid or invalid state
  bool is_valid_ = true;
  /// The cause for being in an invalid state
  String error_message_;
  /// The scheduler the task is bound to
  Scheduler& scheduler_;
  /// The task's identifier
  utils::UUID task_id_ = utils::UUID(nullptr);
};

/**
 * Internal task to remove tasks that finished or are disabled
 *
 * When the task scheduler calls a task's OnDisable function, the task is added
 * to an instance of this task's removal queue. Once activated, it deletes all
 * queued tasks which thereby remove themselves.
 */
class TaskRemovalTask : public Task {
 public:
  TaskRemovalTask(Scheduler& scheduler);
  virtual ~TaskRemovalTask() = default;

  /**
   * Adds a task to the task removal queue
   *
   * \param to_be_removed The task to be deleted and removed
   */
  void add(Task& to_be_removed);

 private:
  /**
   * Goes through all tasks in the removal queue and deletes them
   *
   * \return Always true
   */
  bool Callback();

  /// Queued tasks to be removed
  std::set<Task*> tasks_;
};

}  // namespace tasks
}  // namespace bernd_box
