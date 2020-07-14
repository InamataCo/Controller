#include "dummy_peripheral.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace dummy {

DummyPeripheral::DummyPeripheral() {}

const String& DummyPeripheral::getType() { return type(); }

const String& DummyPeripheral::type() {
  static const String name{"DummyPeripheral"};
  return name;
}

std::shared_ptr<Peripheral> DummyPeripheral::factory(const JsonObjectConst&) {
  return std::make_shared<DummyPeripheral>();
}

bool DummyPeripheral::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

}  // namespace dummy
}  // namespace peripherals
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