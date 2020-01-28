#include "dummyPeriphery.h"

#include "managers/services.h"

namespace bernd_box {
namespace periphery {
namespace peripheries {
namespace dummy {

const String DummyTask::TYPE = "beStupid";


DummyPeriphery::DummyPeriphery() {
  addTaskFactory(DummyTask::TYPE, taskFactory_);
}

const __FlashStringHelper* DummyPeriphery::getType() { return type(); }

const __FlashStringHelper* DummyPeriphery::type() {
  return F("DummyPeriphery");
}

std::shared_ptr<Periphery> DummyPeriphery::factory(const JsonObjectConst&) {
  return std::make_shared<DummyPeriphery>();
}

bool DummyPeriphery::registered_ =
    PeripheryFactory::registerFactory(type(), factory);

// Dummy Task ----------------
DummyTaskFactory DummyPeriphery::taskFactory_ = DummyTaskFactory();

Result DummyTask::execute() {
  const char* who = __PRETTY_FUNCTION__;
  Services::getMqtt().sendError(who, "I'm too dummy");
  return Result::kSuccess;
}

std::unique_ptr<PeripheryTask> DummyTaskFactory::createTask(
    std::shared_ptr<Periphery> periphery, const JsonObjectConst& doc) {
  return std::unique_ptr<PeripheryTask>(new DummyTask(periphery));
}

DummyTask::DummyTask(std::shared_ptr<Periphery> periphery)
    : PeripheryTask(periphery) {}

}  // namespace dummy
}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box
