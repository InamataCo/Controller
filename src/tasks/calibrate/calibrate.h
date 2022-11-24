#pragma once

#include <ArduinoJson.h>

#include "managers/services.h"
#include "peripheral/capabilities/calibrate.h"
#include "tasks/base_task.h"

namespace inamata {
namespace tasks {
namespace calibrate {

class Calibrate : public BaseTask {
 public:
  Calibrate(const JsonObjectConst& parameters, Scheduler& scheduler);
  virtual ~Calibrate() = default;

  const String& getType() const final;
  static const String& type();

  bool TaskCallback() final;

 private:
  static bool registered_;
  static BaseTask* factory(const JsonObjectConst& parameters,
                           Scheduler& scheduler);

  std::shared_ptr<peripheral::capabilities::Calibrate> peripheral_;
};

}  // namespace calibrate
}  // namespace tasks
}  // namespace inamata
