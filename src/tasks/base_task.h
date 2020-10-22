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
  BaseTask(Scheduler& scheduler);
  virtual ~BaseTask() = default;

  virtual const String& getType() const = 0;

  bool OnEnable() final;
  virtual bool OnTaskEnable();

  void OnDisable() final;
  virtual void OnTaskDisable();

  bool isValid() const;
  ErrorResult getError() const;

 protected:
  void setInvalid();
  void setInvalid(const String& error_message);

  static String peripheralNotFoundError(const utils::UUID& uuid);

  static const __FlashStringHelper* peripheral_key_;
  static const __FlashStringHelper* peripheral_key_error_;
  static const __FlashStringHelper* peripheral_not_found_error_;

 private:
  bool is_valid_ = true;
  String error_message_;
  Scheduler& scheduler_;
};

class TaskRemovalTask : public Task {
 private:
  std::set<Task*> tasks_;
  bool Callback();

 public:
  TaskRemovalTask(Scheduler& scheduler);
  virtual ~TaskRemovalTask() = default;
  void add(Task& to_be_removed);
};

}  // namespace tasks
}  // namespace bernd_box
