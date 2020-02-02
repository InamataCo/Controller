#ifndef BERND_BOX_PERIPHERY_ABSTRACTPERIPHERY_H
#define BERND_BOX_PERIPHERY_ABSTRACTPERIPHERY_H

#include <map>

#include "periphery.h"
#include "periphery_task.h"
#include "baseTasks.h"

namespace bernd_box {
namespace periphery {

class AbstractPeriphery : public Periphery {
 public:
  virtual ~AbstractPeriphery() = default;

  TaskFactory& getTaskFactory(const JsonObjectConst& doc) final;
  const bool isValid() final;
  virtual const std::list<String>& getAvaiableTasks();

 protected:
  void addTaskFactory(const String& type, TaskFactory& taskFactory);
  void addTaskFactory(std::map<String, TaskFactory&>& taskFactories);
  void setInvalid();

 private:
  std::map<String, TaskFactory&> task_factories_;
  std::list<String> avaiable_tasks_;
  ErrorTaskFactory error_factory_{ErrorTaskFactory()};
  bool isValid_ = true;
};
}  // namespace periphery
}  // namespace bernd_box

#endif
