#pragma once

#include <memory>

#include "periphery/periphery.h"
#include "periphery/periphery_task.h"
#include "periphery/periphery_task_factory.h"

namespace bernd_box {
namespace periphery {
namespace tasks {

class ReadSensor : public PeripheryTask {
 public:
  ReadSensor(std::shared_ptr<Periphery> periphery,
             const JsonObjectConst& parameters);
  virtual ~ReadSensor() = default;

  const __FlashStringHelper* getType() final;
  static const __FlashStringHelper* type();

  bool OnEnable() final;
  bool Callback() final;

 private:
  static bool registered_;
  static std::unique_ptr<PeripheryTask> factory(
      std::shared_ptr<Periphery> periphery, const JsonObjectConst& parameters);
};

}  // namespace tasks
}  // namespace periphery
}  // namespace bernd_box
