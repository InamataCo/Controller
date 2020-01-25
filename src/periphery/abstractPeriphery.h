#ifndef BERND_BOX_PERIPHERY_ABSTRACTPERIPHERY_H
#define BERND_BOX_PERIPHERY_ABSTRACTPERIPHERY_H

#include "library/library.h"
#include "managers/io.h"
#include "periphery.h"

namespace bernd_box {
namespace periphery {
using namespace bernd_box::library;
class AbstractPeriphery : public Periphery {
 public:
  AbstractPeriphery(Library& library, const String name);
  Result executeTask(const JsonObjectConst& doc);
  Library& getLibrary(); 

 protected:
  void addTaskFactory(const String& type, TaskFactory& taskFactory);
  void addTaskFactory(std::map<String, TaskFactory&>& taskFactories);

 private:
  std::map<String, TaskFactory&> taskFactories_;
  const String name_;
  Library& library_;
};
}  // namespace periphery
}  // namespace bernd_box

#endif