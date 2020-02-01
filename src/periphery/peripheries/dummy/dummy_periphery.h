#ifndef BERND_BOX_PERIPHERY_PERIPHERIES_DUMMY_DUMMYPERIPHERIE_H
#define BERND_BOX_PERIPHERY_PERIPHERIES_DUMMY_DUMMYPERIPHERIE_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "library/library.h"
#include "managers/services.h"
#include "periphery/abstract_periphery.h"
#include "periphery/periphery.h"
#include "periphery/periphery_factory.h"
#include "periphery/periphery_task.h"

namespace bernd_box {
namespace periphery {
namespace peripheries {
namespace dummy {

class DummyTaskFactory : public TaskFactory {
 public:
  std::unique_ptr<PeripheryTask> createTask(
      std::shared_ptr<Periphery> periphery, const JsonObjectConst& doc);
};

class DummyPeriphery : public AbstractPeriphery {
 public:
  DummyPeriphery();
  virtual ~DummyPeriphery() = default;
  const String& getType() final;
  static const String& type();

 private:
  static std::shared_ptr<Periphery> factory(const JsonObjectConst&);

  static bool registered_;
  static DummyTaskFactory taskFactory_;
};

class DummyTask : public PeripheryTask {
 public:
  DummyTask(std::shared_ptr<Periphery> periphery);
  virtual ~DummyTask() = default;

  Result execute() final;
  static const String TYPE;
};
}  // namespace dummy
}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box

#endif
