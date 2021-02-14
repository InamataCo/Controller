#pragma once

#include <memory>

#include "ArduinoJson.h"
#include "tasks/get_values_task/get_values_task.h"
#include "utils/value_unit.h"

namespace bernd_box {
namespace tasks {
namespace alert_sensor {

class AlertSensor : public get_values_task::GetValuesTask {
 public:
  enum class TriggerType { kRising, kFalling, kEither };

  AlertSensor(const JsonObjectConst& parameters, Scheduler& scheduler);
  virtual ~AlertSensor() = default;

  const String& getType() const final;
  static const String& type();

  bool TaskCallback() final;

  /**
   * Sets the trigger type
   *
   * Used to evaluate which type of crossing the threshold will cause an alert
   * to be sent. Sets kInvalid type on unknown type.
   *
   * @param trigger_type The type as string [rising, falling, either]
   * @return true on valid trigger type
   */
  bool setTriggerType(const String& trigger_type);

  /**
   * Set the Trigger Type object
   *
   *
   * @see setTriggerType(const String&)
   * @param type The type as a trigger type enum
   */
  void setTriggerType(const TriggerType type);

  /**
   * Returns the current trigger type
   *
   * @see setTriggerType(const String&)
   * @return TriggerType
   */
  TriggerType getTriggerType();

  TriggerType triggerType2Enum(const String& trigger_type);
  const __FlashStringHelper* triggerType2String(TriggerType trigger_type);

 private:
  bool sendAlert(TriggerType trigger_type);
  bool isRisingThreshold(const float value);
  bool isFallingThreshold(const float value);

  static bool registered_;
  static BaseTask* factory(const JsonObjectConst& parameters,
                           Scheduler& scheduler);

  static const std::map<TriggerType, const __FlashStringHelper*>
      trigger_type_strings_;

  /// The data point type to trigger on
  utils::UUID data_point_type_;

  /// Default interval to poll the sensor with
  const std::chrono::milliseconds default_interval_{100};

  /// The direction of the sensor value crossing the trigger to send an alert
  TriggerType trigger_type_;

  /// The threshold to create an alert for
  float threshold_;

  /// Last measured sensor value
  float last_value_;
};

}  // namespace alert_sensor
}  // namespace tasks
}  // namespace bernd_box
