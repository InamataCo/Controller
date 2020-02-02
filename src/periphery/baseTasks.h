#ifndef BERND_BOX_PERIPHERY_BASETASK_H
#define BERND_BOX_PERIPHERY_BASETASK_H

#include "periphery.h"
#include "periphery_task.h"

#define TASK_ERROR "error"
#define ERROR_TASK_ERROR_MESSAGE \
  "The periphery does not support the requested task."
  
#define TASK_LIST_OPERATIONS "list_operations"
#define TASK_LIST_RESULT_NODE "result"
#define TASK_LIST_TYPE_NODE "type"

namespace bernd_box {
namespace periphery {

class ErrorTaskFactory : public TaskFactory {
 public:
  virtual ~ErrorTaskFactory() = default;
  PeripheryTask& createTask(std::shared_ptr<Periphery> periphery,
      const JsonObjectConst& doc);
};

class ErrorTask : public PeripheryTask {
 public:
  ErrorTask(std::shared_ptr<Periphery> periphery);
  virtual ~ErrorTask() = default;

  bool Callback() final;
  static const String TYPE;
};


class ListOperationsTaskFactory : public TaskFactory {
 public:
  virtual ~ListOperationsTaskFactory() = default;
  PeripheryTask& createTask(std::shared_ptr<Periphery> periphery,
      const JsonObjectConst& doc);
};

class ListOperationsTask : public PeripheryTask {
 public:
  ListOperationsTask(std::shared_ptr<Periphery> periphery);
  virtual ~ListOperationsTask() = default;

  bool Callback() final;
  static const String TYPE;
};

}  // namespace periphery
}  // namespace bernd_box
#endif