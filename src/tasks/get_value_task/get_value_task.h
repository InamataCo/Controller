#pragma once

#include <ArduinoJson.h>

#include <memory>

#include "managers/services.h"
#include "peripheral/capabilities/get_value.h"
#include "peripheral/peripheral.h"
#include "tasks/base_task.h"
#include "utils/uuid.h"

namespace bernd_box {
namespace tasks {

/**
 * Abstract class that implements getting a peripheral which supports the
 * GetValue capability for a given name.
 */
class GetValueTask : public BaseTask {
 public:
  GetValueTask(const JsonObjectConst& parameters, Scheduler& scheduler);
  virtual ~GetValueTask() = default;

  std::shared_ptr<peripheral::capabilities::GetValue> getPeripheral();
  const UUID& getPeripheralUUID() const;

 protected:
  // JSON keys and error messages
  static const __FlashStringHelper* threshold_key_;
  static const __FlashStringHelper* threshold_key_error_;
  static const __FlashStringHelper* trigger_type_key_;
  static const __FlashStringHelper* trigger_type_key_error_;
  static const __FlashStringHelper* interval_ms_key_;
  static const __FlashStringHelper* interval_ms_key_error_;
  static const __FlashStringHelper* duration_ms_key_;
  static const __FlashStringHelper* duration_ms_key_error_;

 private:
  std::shared_ptr<peripheral::capabilities::GetValue> peripheral_;
  UUID peripheral_uuid_;
};

}  // namespace tasks
}  // namespace bernd_box
