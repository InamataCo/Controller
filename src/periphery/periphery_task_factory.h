#pragma once

#include <ArduinoJson.h>

#include <map>
#include <memory>

#include "managers/mqtt.h"
#include "periphery.h"
#include "periphery_task.h"

namespace bernd_box {
namespace periphery {

class PeripheryTaskFactory {
 public:
  PeripheryTaskFactory(Mqtt& mqtt);

  using Factory = std::unique_ptr<PeripheryTask> (*)(
      std::shared_ptr<Periphery> periphery, const JsonObjectConst& parameters);

  static bool registerTask(const String& type, Factory factory);

  bool createTask(std::shared_ptr<Periphery> periphery,
                  const JsonObjectConst& parameter);

  bool stopTask(const JsonObjectConst& parameter);

 private:
  const int getNextTaskId();

  static std::map<String, Factory> factories_;
  std::map<int, std::unique_ptr<PeripheryTask>> tasks_;

  Mqtt& mqtt_;

  int next_task_id_ = 0;
};

}  // namespace periphery
}  // namespace bernd_box
