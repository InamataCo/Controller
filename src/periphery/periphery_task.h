#ifndef BERND_BOX_PERIPHERY_TASK_H
#define BERND_BOX_PERIPHERY_TASK_H

#include <Arduino.h>

#include <memory>
#include <set>

// #include "managers/io.h"
#include "TaskSchedulerDeclarations.h"
#include "managers/io_types.h"
#include "periphery.h"

namespace bernd_box {
namespace periphery {

class PeripheryTask : public Task {
 private:
  std::shared_ptr<Periphery> periphery_;
  Scheduler& scheduler_;

 public:
  PeripheryTask(std::shared_ptr<Periphery> periphery);
  virtual ~PeripheryTask() = default;

  virtual void OnDisable() final;
  virtual void OnTaskDisable();

  std::shared_ptr<Periphery> getPeriphery();
};

class TaskFactory {
 public:
  virtual PeripheryTask& createTask(std::shared_ptr<Periphery> periphery,
                                    const JsonObjectConst& parameter) = 0;
};

class PeripheryTaskRemovalTask : public Task {
 private:
  std::set<PeripheryTask*> tasks_;
  bool Callback();

 public:
  PeripheryTaskRemovalTask(Scheduler& scheduler);
  virtual ~PeripheryTaskRemovalTask() = default;
  void add(PeripheryTask& to_be_removed);
};

}  // namespace periphery
}  // namespace bernd_box

#endif
