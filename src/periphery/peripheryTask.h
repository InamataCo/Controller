#ifndef BERND_BOX_PERIPHERY_TASK_H
#define BERND_BOX_PERIPHERY_TASK_H

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>

#include "managers/io.h"
#include "periphery.h"

namespace bernd_box {
namespace periphery {

class PeripheryTask /*: public Task*/ {
 private:
  std::shared_ptr<Periphery> periphery_;

 protected:
  

 public:
  PeripheryTask(std::shared_ptr<Periphery> periphery);
  virtual ~PeripheryTask() = default;

  std::shared_ptr<Periphery> getPeriphery();
  virtual Result execute() = 0;
};

class TaskFactory {
 public:
  virtual std::unique_ptr<PeripheryTask> createTask(std::shared_ptr<Periphery> periphery,
      const JsonObjectConst& parameter) = 0;
};

}  // namespace periphery
}  // namespace bernd_box

#endif