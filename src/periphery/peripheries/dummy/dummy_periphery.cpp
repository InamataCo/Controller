#include "dummy_periphery.h"

namespace bernd_box {
namespace periphery {
namespace peripheries {
namespace dummy {

const String DummyTask::TYPE = "beStupid";

DummyPeriphery::DummyPeriphery() {
  addTaskFactory(DummyTask::TYPE, taskFactory_);
}

const String& DummyPeriphery::getType() { return type(); }

const String& DummyPeriphery::type() {
  static const String name{"DummyPeriphery"};
  return name;
}

std::shared_ptr<Periphery> DummyPeriphery::factory(const JsonObjectConst&) {
  return std::make_shared<DummyPeriphery>();
}

bool DummyPeriphery::registered_ =
    PeripheryFactory::registerFactory(type(), factory);

// Dummy Task ----------------
DummyTaskFactory DummyPeriphery::taskFactory_ = DummyTaskFactory();

bool DummyTask::Callback() {
  const char* who = __PRETTY_FUNCTION__;
  Services::getMqtt().sendError(who, "I'm too dummy");
  disable();
  return false;
}

PeripheryTask& DummyTaskFactory::createTask(
    std::shared_ptr<Periphery> periphery, const JsonObjectConst& doc) {
  return *new DummyTask(periphery);
}

DummyTask::DummyTask(std::shared_ptr<Periphery> periphery)
    : PeripheryTask(periphery) {}

}  // namespace dummy
}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box
