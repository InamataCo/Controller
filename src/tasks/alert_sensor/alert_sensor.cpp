#include "alert_sensor.h"

#include "tasks/task_factory.h"

namespace bernd_box {
namespace tasks {
namespace alert_sensor {

AlertSensor::AlertSensor(const ServiceGetters& services,
                         const JsonObjectConst& parameters,
                         Scheduler& scheduler)
    : GetValuesTask(parameters, scheduler) {
  if (!isValid()) {
    return;
  }

  server_ = services.get_server();
  if (server_ == nullptr) {
    setInvalid(services.server_nullptr_error_);
    return;
  }

  // Get the type of trigger [rising, falling, either]
  JsonVariantConst trigger_type = parameters[trigger_type_key_];
  if (trigger_type.isNull() || !trigger_type.is<char*>()) {
    setInvalid(trigger_type_key_error_);
    return;
  }
  setTriggerType(trigger_type.as<String>());

  // Get the trigger threshold
  JsonVariantConst threshold = parameters[threshold_key_];
  if (!trigger_type.is<int>()) {
    setInvalid(threshold_key_error_);
    return;
  }
  threshold_ = threshold;

  // Optionally get the interval with which to poll the sensor [default: 100ms]
  JsonVariantConst interval_ms = parameters[interval_ms_key_];
  if (interval_ms.isNull()) {
    setInterval(std::chrono::milliseconds(default_interval_).count());
  } else if (interval_ms.is<unsigned int>()) {
    setInterval(interval_ms);
  } else {
    setInvalid(interval_ms_key_error_);
    return;
  }

  // Optionally get the duration for which to poll the sensor [default: forever]
  JsonVariantConst duration_ms = parameters[duration_ms_key_];
  if (duration_ms.isNull()) {
    setIterations(TASK_FOREVER);
  } else if (duration_ms.is<unsigned int>()) {
    setIterations(duration_ms.as<unsigned int>() / getInterval());
  } else {
    setInvalid(duration_ms_key_error_);
    return;
  }

  data_point_type_ =
      utils::UUID(parameters[utils::ValueUnit::data_point_type_key]);
  if (!data_point_type_.isValid()) {
    setInvalid(utils::ValueUnit::data_point_type_key_error);
    return;
  }

  enable();
}

const String& AlertSensor::getType() const { return type(); }

const String& AlertSensor::type() {
  static const String name{"AlertSensor"};
  return name;
}

bool AlertSensor::TaskCallback() {
  auto result = getPeripheral()->getValues();
  if (result.error.isError()) {
    setInvalid(result.error.toString());
    return false;
  }

  // Find the specified data point type. Uses first found. Error if none found
  auto match_unit = [&](const utils::ValueUnit& value_unit) {
    return value_unit.data_point_type == data_point_type_;
  };
  const auto trigger_value_unit =
      std::find_if(result.values.cbegin(), result.values.cend(), match_unit);
  if (trigger_value_unit == result.values.end()) {
    setInvalid(String(F("Data point type not found: ")) +
               data_point_type_.toString());
    return false;
  }

  // Check the flank type if it should trigger
  if (isRisingThreshold(trigger_value_unit->value)) {
    if (trigger_type_ == TriggerType::kRising ||
        trigger_type_ == TriggerType::kEither) {
      sendAlert(TriggerType::kRising);
    }
  } else if (isFallingThreshold(trigger_value_unit->value)) {
    if (trigger_type_ == TriggerType::kFalling ||
        trigger_type_ == TriggerType::kEither) {
      sendAlert(TriggerType::kFalling);
    }
  }

  // Store the current value for the next iteration
  last_value_ = trigger_value_unit->value;
  return true;
}

bool AlertSensor::setTriggerType(const String& type) {
  for (auto trigger_type_string : trigger_type_strings_) {
    if (type == trigger_type_string.second) {
      trigger_type_ = trigger_type_string.first;
      return true;
    }
  }

  return false;
}

void AlertSensor::setTriggerType(const TriggerType type) {
  trigger_type_ = type;
}

AlertSensor::TriggerType AlertSensor::getTriggerType() { return trigger_type_; }

bool AlertSensor::sendAlert(TriggerType trigger_type) {
  if (trigger_type == TriggerType::kRising ||
      trigger_type == TriggerType::kFalling) {
    DynamicJsonDocument doc(BB_JSON_PAYLOAD_SIZE);
    doc[threshold_key_] = threshold_;

    auto trigger_type_string = trigger_type_strings_.find(trigger_type);
    if (trigger_type_string != trigger_type_strings_.end()) {
      doc[trigger_type_key_] = trigger_type_string->second;
    } else {
      return false;
    }

    doc[peripheral_key_] = getPeripheralUUID().toString();

    server_->send(type(), doc);
    return true;
  }

  return false;
}

bool AlertSensor::isRisingThreshold(const float value) {
  return value > threshold_ && last_value_ < threshold_;
}

bool AlertSensor::isFallingThreshold(const float value) {
  return value < threshold_ && last_value_ > threshold_;
}

bool AlertSensor::registered_ = TaskFactory::registerTask(type(), factory);

BaseTask* AlertSensor::factory(const ServiceGetters& services,
                               const JsonObjectConst& parameters,
                               Scheduler& scheduler) {
  return new AlertSensor(services, parameters, scheduler);
}

const std::map<AlertSensor::TriggerType, const __FlashStringHelper*>
    AlertSensor::trigger_type_strings_{{TriggerType::kRising, F("rising")},
                                       {TriggerType::kFalling, F("falling")},
                                       {TriggerType::kEither, F("either")}};

}  // namespace alert_sensor
}  // namespace tasks
}  // namespace bernd_box
