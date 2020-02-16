#include "dummy_periphery.h"

namespace bernd_box {
namespace peripheral {
namespace peripheries {
namespace dummy {

DummyPeriphery::DummyPeriphery() {}

const String& DummyPeriphery::getType() { return type(); }

const String& DummyPeriphery::type() {
  static const String name{"DummyPeriphery"};
  return name;
}

std::shared_ptr<Peripheral> DummyPeriphery::factory(const JsonObjectConst&) {
  return std::make_shared<DummyPeriphery>();
}

bool DummyPeriphery::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

}  // namespace dummy
}  // namespace peripheries
}  // namespace peripheral

namespace tasks {

const String DummyTask::TYPE = "beStupid";

DummyTask::~DummyTask() { Serial.println("Deleting DummyTask"); }

bool DummyTask::Callback() {
  const char* who = __PRETTY_FUNCTION__;
  Serial.println("Too dummy");
  Services::getMqtt().send(who, "I'm too dummy");
  return true;
}

void DummyTask::OnTaskDisable() {
  Serial.println("I'm getting disabled... going to disappear :(");
}

DummyTask::DummyTask(Scheduler& scheduler) : BaseTask(scheduler) {}

bool DummyTask::registered_ = TaskFactory::registerTask(type(), factory);

BaseTask* DummyTask::factory(const JsonObjectConst& parameters,
                             Scheduler& scheduler) {
  BaseTask* task = new DummyTask(scheduler);
  task->setIterations(1);
  return task;
}

const __FlashStringHelper* DummyTask::type() { return F("DummyTask"); }
const __FlashStringHelper* DummyTask::getType() { return type(); }

}  // namespace tasks
}