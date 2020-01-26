#ifndef BERND_BOX_PERIPHERY_PERIPHERIES_DUMMY_DUMMYPERIPHERIE_H
#define BERND_BOX_PERIPHERY_PERIPHERIES_DUMMY_DUMMYPERIPHERIE_H

#include "library/library.h"
#include "periphery/abstractPeriphery.h"
#include "periphery/periphery.h"
#include "periphery/peripheryTask.h"

#include <Arduino.h>
#include <ArduinoJson.h>

namespace bernd_box {
namespace periphery {
namespace peripheries {
namespace dummy {

using namespace bernd_box::library;
using namespace bernd_box::periphery;

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
  static const String TYPE;

 private:
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