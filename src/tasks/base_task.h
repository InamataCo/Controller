#pragma once

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>

#include <functional>
#include <set>

namespace bernd_box {
namespace tasks {

class BaseTask : public Task {
 public:
  BaseTask(Scheduler& scheduler);
  virtual ~BaseTask() = default;

  virtual const String& getType() = 0;

  void OnDisable() final;
  virtual void OnTaskDisable();

  bool isValid();

 protected:
  void setInvalid();

 private:
  bool is_valid_ = true;
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
