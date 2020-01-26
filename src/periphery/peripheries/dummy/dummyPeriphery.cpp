#include "dummyPeriphery.h"

#include "managers/services.h"

namespace bernd_box {
namespace periphery {
namespace peripheries {
namespace dummy {

const String DummyTask::TYPE = "beStupid";
const String DummyPeriphery::TYPE = "Dummy";

DummyTaskFactory DummyPeriphery::taskFactory_ = DummyTaskFactory();

DummyPeriphery::DummyPeriphery() {
  addTaskFactory(DummyTask::TYPE, taskFactory_);
}
const String& DummyPeriphery::getType() { return DummyPeriphery::TYPE; }

Result DummyTask::execute() {
  const char* who = __PRETTY_FUNCTION__;
  Services::getMQTT().sendError(who, "I'm too dummy");
  return Result::kSuccess;
}

std::unique_ptr<PeripheryTask> DummyTaskFactory::createTask(
    std::shared_ptr<Periphery> periphery, const JsonObjectConst& doc) {
  return std::unique_ptr<PeripheryTask>(new DummyTask(periphery));
}

DummyTask::DummyTask(std::shared_ptr<Periphery> periphery) : PeripheryTask(periphery) { }

}  // namespace dummy
}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box