#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "library/library.h"
#include "managers/services.h"
#include "peripheral/peripheral.h"
#include "peripheral/periphery_factory.h"
#include "tasks/base_task.h"

namespace bernd_box {
namespace peripheral {
namespace peripheries {
namespace dummy {

class DummyPeriphery : public Peripheral {
 public:
  DummyPeriphery();
  virtual ~DummyPeriphery() = default;
  const String& getType() final;
  static const String& type();

 private:
  static std::shared_ptr<Peripheral> factory(const JsonObjectConst&);

  static bool registered_;
};
}  // namespace dummy
}  // namespace peripheries
}  // namespace peripheral

namespace tasks {
class DummyTask : public BaseTask {
 public:
  DummyTask(Scheduler& scheduler);
  virtual ~DummyTask();

  bool Callback() final;
  void OnTaskDisable() final;

  const __FlashStringHelper* getType();
  static const __FlashStringHelper* type();
  static const String TYPE;

 private:
  static bool registered_;
  static BaseTask* factory(const JsonObjectConst& parameters,
                           Scheduler& scheduler);
};
}  // namespace tasks
}  // namespace bernd_box