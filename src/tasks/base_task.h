#pragma once

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>

#include <functional>

namespace bernd_box {
namespace tasks {

class BaseTask : public Task {
 public:
  /// Callback to remove task after it has disabled itself
  using RemoveCallback = std::function<void(const int id)>;

  BaseTask(Scheduler& scheduler, RemoveCallback remove_callback);
  virtual ~BaseTask() = default;

  virtual const __FlashStringHelper* getType() = 0;

  void OnDisable() final;
  virtual void OnTaskDisable();

  bool isValid();

 protected:
  void setInvalid();

 private:
  bool is_valid_ = true;
};

}  // namespace tasks
}  // namespace bernd_box
