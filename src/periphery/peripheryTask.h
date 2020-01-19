#ifndef BERND_BOX_PERIPHERY_TASK_H
#define BERND_BOX_PERIPHERY_TASK_H

#include "TaskSchedulerDeclarations.h"
#include "managers/io.h"
#include "periphery.h"

#include <Arduino.h>

// STD C++ includes placed after Arduino.h
#include <algorithm>
#include <cmath>
#include <map>
#include <string>

namespace bernd_box {
namespace periphery {

class PeripheryTask {
 public:
  virtual const String& getType();
  virtual Result execute(Periphery& periphery);
};

class TaskFactory {
 public:
  virtual PeripheryTask& createTask(const JsonObjectConst& doc);
};

}  // namespace periphery
}  // namespace bernd_box

#endif