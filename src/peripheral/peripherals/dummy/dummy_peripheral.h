#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "peripheral/peripheral.h"
#include "peripheral/peripheral_factory.h"
#include "tasks/base_task.h"
#include "tasks/task_factory.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace dummy {

/**
 * A peripheral without a purpose
 */
class DummyPeripheral : public Peripheral {
 public:
  DummyPeripheral();
  virtual ~DummyPeripheral();

  const String& getType() const final;
  static const String& type();

 private:
  static std::shared_ptr<Peripheral> factory(const JsonObjectConst&);

  static bool registered_;
};
}  // namespace dummy
}  // namespace peripherals
}  // namespace peripheral

namespace tasks {
class DummyTask : public BaseTask {
 public:
  DummyTask(Scheduler& scheduler);
  virtual ~DummyTask();

  bool Callback() final;
  void OnTaskDisable() final;

  const String& getType() const final;
  static const String& type();

 private:
  static bool registered_;
  static BaseTask* factory(const JsonObjectConst& parameters,
                           Scheduler& scheduler);
};
}  // namespace tasks
}  // namespace bernd_box