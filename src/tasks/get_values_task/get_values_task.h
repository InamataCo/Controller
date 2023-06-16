#pragma once

#include <ArduinoJson.h>

#include <memory>

#include "peripheral/capabilities/get_values.h"
#include "tasks/base_task.h"
#include "utils/uuid.h"

namespace inamata {
namespace tasks {
namespace get_values_task {

/**
 * Abstract class that implements getting a peripheral which supports the
 * GetValue capability for a given name.
 */
class GetValuesTask : public BaseTask {
 public:
  GetValuesTask(const JsonObjectConst& parameters, Scheduler& scheduler);
  virtual ~GetValuesTask() = default;

  std::shared_ptr<peripheral::capabilities::GetValues> getPeripheral();
  const utils::UUID& getPeripheralUUID() const;

  /**
   * Make a JSON object with the value units and UUID from the peripheral
   *
   * \param telemetry The JSON object to add the value units and UUID to
   * \return An object with the error, if one occured
   */
  ErrorResult packageValues(JsonObject& telemetry);

  static const __FlashStringHelper* threshold_key_;
  static const __FlashStringHelper* threshold_key_error_;
  static const __FlashStringHelper* trigger_type_key_;
  static const __FlashStringHelper* trigger_type_key_error_;
  static const __FlashStringHelper* interval_ms_key_;
  static const __FlashStringHelper* interval_ms_key_error_;
  static const __FlashStringHelper* duration_ms_key_;
  static const __FlashStringHelper* duration_ms_key_error_;

 private:
  std::shared_ptr<peripheral::capabilities::GetValues> peripheral_;
  utils::UUID peripheral_uuid_;
};

}  // namespace get_values_task
}  // namespace tasks
}  // namespace inamata
