#ifndef BERND_BOX_PERIPHERY_TASK_H
#define BERND_BOX_PERIPHERY_TASK_H

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>

#include <memory>
#include <set>

#include "managers/io_types.h"
#include "periphery.h"


#define PERIPHERY_TASK_NAME_NODE "name"
#define PERIPHERY_TASK_TYPE_NODE "type"
#define PERIPHERY_TASK_PARAMETER_NODE "parameter"
#define PERIPHERY_TASK_RESPONSE_CODE_NODE "responseCode"
#define PERIPHERY_TASK_TASK_ID_NODE "taskID"
#define PERIPHERY_TASK_RESULT_NODE "result"

namespace bernd_box {
namespace periphery {

class PeripheryTask : public Task {
 public:
  PeripheryTask(std::shared_ptr<Periphery> periphery);
  virtual ~PeripheryTask() = default;

  void OnDisable() final;
  virtual void OnTaskDisable();

  std::shared_ptr<Periphery> getPeriphery();
  virtual const __FlashStringHelper* getType() = 0;

 private:
  std::shared_ptr<Periphery> periphery_;
  Scheduler& scheduler_;
};

class TaskFactory {
 public:
  virtual PeripheryTask& createTask(std::shared_ptr<Periphery> periphery,
                                    const JsonObjectConst& parameter) = 0;
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

}  // namespace periphery
}  // namespace bernd_box

#endif
