#include "dummyPeriphery.h"

namespace bernd_box {
namespace periphery {
namespace peripheries {
namespace dummy {

const String DummyTask::TYPE = "beStupid";
const String DummyPeriphery::TYPE = "Dummy";

DummyPeriphery::DummyPeriphery(Library& library, const String name)
    : AbstractPeriphery(library, name) {
  TaskFactory& ref = *new DummyTaskFactory();
  addTaskFactory(DummyTask::TYPE, ref);
}
const String& DummyPeriphery::getType() { return DummyPeriphery::TYPE; }

const String& DummyTask::getType() { return DummyTask::TYPE; }

Result DummyTask::execute(Periphery& periphery) {
  const char* who = __PRETTY_FUNCTION__;
  Mqtt& mqtt = periphery.getLibrary().getMQTT();

  mqtt.sendError(who, "I'm too dummy");
  return Result::kSuccess;
}

PeripheryTask& DummyTaskFactory::createTask(const JsonObjectConst& doc) {
  return *new DummyTask();
}

}  // namespace dummy
}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box