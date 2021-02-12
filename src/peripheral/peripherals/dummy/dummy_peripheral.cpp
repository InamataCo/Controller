#include "dummy_peripheral.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace dummy {

DummyPeripheral::DummyPeripheral() {
  Serial.println("Creating DummyPeripheral");
}

DummyPeripheral::~DummyPeripheral() {
  Serial.println("Deleting DummyPeripheral");
}

const String& DummyPeripheral::getType() const { return type(); }

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

DummyTask::~DummyTask() { Serial.println("Deleting DummyTask"); }

void DummyTask::TaskCallback() {
  Serial.println("Too dummy");
}

void DummyTask::OnTaskDisable() {
  Serial.println(F("I'm getting disabled... going to disappear :("));
}

DummyTask::DummyTask(Scheduler& scheduler) : BaseTask(scheduler) {}

bool DummyTask::registered_ = TaskFactory::registerTask(type(), factory);

BaseTask* DummyTask::factory(const JsonObjectConst& parameters,
                             Scheduler& scheduler) {
  BaseTask* task = new DummyTask(scheduler);
  task->setIterations(1);
  return task;
}

const String& DummyTask::type() {
  static const String name{"DummyTask"};
  return name;
}

const String& DummyTask::getType() const { return type(); }

}  // namespace tasks
}  // namespace bernd_box