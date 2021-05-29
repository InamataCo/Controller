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
   * If the task ID is not defined, it is marked as a system task which won't
   * be deleted by the task remover.
   * 
   * \param scheduler The scheduler that executes the tasks
   * \param uuid Unique identifier created locally
   */
  BaseTask(Scheduler& scheduler, utils::UUID task_id = utils::UUID(nullptr));

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
   * Called by the task scheduler when the task is enabled and checks if the
   * task is valid
   * 
   * \see OnTaskEnable()
   *
   * \return True if the task is valid
   */
  bool OnEnable() final;

  /**
   * For derived classes to perform init tasks and called when the task is
   * enabled
   *
   * \see OnEnable()
   * 
   * \return True if the derived task inited successfully
   */
  virtual bool OnTaskEnable();

  /**
   * Called by the task scheduler when the task is to be executed
   *
   * \see TaskCallback()
   * 
   * \return True if the task performed work
   */
  bool Callback() final;

  /**
   * For derived classes to perform execution logic.
   *
   * Use the setInvalid() functions to set error states. The task is cleaned up
   * and the error reported if it is not valid before or after being executed.
   * 
   * \see Callback()
   * 
   * \return True to continue, false to end normally
   */
  virtual bool TaskCallback() = 0;

  /**
   * Called by the task scheduler and triggers task removal
   *
   * Contains a static TaskRemovalTask which is tasked with removing disabled
   * tasks.
   * 
   * \see OnTaskDisable()
   */
  void OnDisable() final;

  /**
   * For derived classes to perform clean up
   * 
   * \see OnDisable()
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

  /**
   * Checks if it is a system task
   * 
   * \return True if it is a system task
   */
  bool isSystemTask() const;

  /**
   * Sets the callback which accepts tasks to be removed
   *
   * \param callback The function to call to add a task to the removal queue
   */
  static void setTaskRemovalCallback(std::function<void(Task&)> callback);

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
  /// Add task to removal queue callback
  static std::function<void(Task&)> task_removal_callback_;
};

}  // namespace tasks
}  // namespace bernd_box
