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
  Serial.println("Too dummy");
  Services::getMqtt().send(who, "I'm too dummy");
  return true;
}

void DummyTask::OnTaskDisable(){  
  Serial.println("I'm getting disabled... going to disappear :(");
}

PeripheryTask& DummyTaskFactory::createTask(
    std::shared_ptr<Periphery> periphery, const JsonObjectConst& doc) {
  PeripheryTask& task = *new DummyTask(periphery);
  task.setIterations(1);
  return task;
}

DummyTask::DummyTask(std::shared_ptr<Periphery> periphery)
    : PeripheryTask(periphery) {}

}  // namespace dummy
}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box
