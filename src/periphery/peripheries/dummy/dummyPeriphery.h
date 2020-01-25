#ifndef BERND_BOX_PERIPHERY_PERIPHERIES_DUMMY_DUMMYPERIPHERIE_H
#define BERND_BOX_PERIPHERY_PERIPHERIES_DUMMY_DUMMYPERIPHERIE_H

#include "library/library.h"
#include "periphery/abstractPeriphery.h"
#include "periphery/periphery.h"
#include "periphery/peripheryTask.h"

#include <ArduinoJson.h>
#include <Arduino.h>

namespace bernd_box {
namespace periphery {
namespace peripheries {
namespace dummy {

using namespace bernd_box::library;
using namespace bernd_box::periphery;

class DummyPeriphery : public AbstractPeriphery {
 public:
  DummyPeriphery(Library& library, const String name);
  const String& getType() final;
  static const String TYPE;
};

class DummyTask : public PeripheryTask {
 public:
  const String& getType() final;
  Result execute(Periphery& periphery);
  static const String TYPE;
};

class DummyTaskFactory : public TaskFactory {
 public:
  PeripheryTask& createTask(const JsonObjectConst& doc);
};

}  // namespace dummy
}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box

#endif