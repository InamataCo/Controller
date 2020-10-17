#pragma once

#include "base_task.h"

namespace bernd_box {
namespace tasks {

class InvalidTask : public BaseTask {
 public:
  InvalidTask(Scheduler& scheduler);
  InvalidTask(Scheduler& scheduler, const String& error);
  virtual ~InvalidTask() = default;

  bool Callback() final;

  const String& getType() const;
  static const String& type();
};

}  // namespace tasks

}  // namespace bernd_box
