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

 protected:
  void addTaskFactory(const String& type, TaskFactory& taskFactory);
  void addTaskFactory(std::map<String, TaskFactory&>& taskFactories);
  void setInvalid();

 private:
  std::map<String, TaskFactory&> taskFactories_;
  ErrorTaskFactory errorFactory_{ErrorTaskFactory()};
  bool isValid_ = true;
};
}  // namespace periphery
}  // namespace bernd_box

#endif
