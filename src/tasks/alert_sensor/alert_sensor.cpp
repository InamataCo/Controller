#include "alert_sensor.h"

namespace bernd_box {
namespace tasks {

AlertSensor::AlertSensor(const JsonObjectConst& parameters,
                         Scheduler& scheduler)
    : GetValueTask(parameters, scheduler) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  // Get the type of trigger [rising, falling, either]
  JsonVariantConst trigger_type = parameters[F("trigger_type")];
  if (trigger_type.isNull() || !trigger_type.is<char*>()) {
    Services::getMqtt().sendError(who,
                                  F("Missing property: trigger_type (string)"));
    setInvalid();
    return;
  }
  setTriggerType(trigger_type.as<String>());

  // Get the trigger threshold
  JsonVariantConst threshold = parameters[F("threshold")];
  if (!trigger_type.is<int>()) {
    Services::getMqtt().sendError(who, F("Missing property: threshold (int)"));
    setInvalid();
    return;
  }
  threshold_ = threshold;

  // Optionally get the interval with which to poll the sensor [default: 100ms]
  JsonVariantConst interval_ms = parameters[F("interval_ms")];
  if (interval_ms.isNull()) {
    setInterval(std::chrono::milliseconds(default_interval_).count());
  } else if (interval_ms.is<unsigned int>()) {
    setInterval(interval_ms);
  } else {
    Services::getMqtt().sendError(
        who, F("Wrong type for optional property: interval_ms (unsigned int)"));
    setInvalid();
    return;
  }

  // Optionally get the duration for which to poll the sensor [default: forever]
  JsonVariantConst duration_ms = parameters[F("duration_ms")];
  if (duration_ms.isNull()) {
    setIterations(TASK_FOREVER);
  } else if (duration_ms.is<unsigned int>()) {
    setIterations(duration_ms.as<unsigned int>() / getInterval());
  } else {
    Services::getMqtt().sendError(
        who, F("Wrong type for optional property: duration_ms (unsigned int)"));
    setInvalid();
    return;
  }

  enable();
}

const __FlashStringHelper* AlertSensor::getType() { return type(); }

const __FlashStringHelper* AlertSensor::type() { return F("AlertSensor"); }

bool AlertSensor::OnEnable() { return true; }

bool AlertSensor::Callback() {
  float value = getPeriphery()->getValue();

  if (isRisingThreshold(value)) {
    if (trigger_type_ == TriggerType::kRising ||
        trigger_type_ == TriggerType::kEither) {
      sendAlert(TriggerType::kRising);
    }
  } else if (isFallingThreshold(value)) {
    if (trigger_type_ == TriggerType::kFalling ||
        trigger_type_ == TriggerType::kEither) {
      sendAlert(TriggerType::kFalling);
    }
  }

  last_value_ = value;
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
    DynamicJsonDocument doc(BB_MQTT_JSON_PAYLOAD_SIZE);
    doc[F("threshold")] = threshold_;

    auto trigger_type_string = trigger_type_strings_.find(trigger_type);
    if (trigger_type_string != trigger_type_strings_.end()) {
      doc[F("trigger_type")] = trigger_type_string->second;
    } else {
      return false;
    }

    doc[F("periphery_name")] = getPeripheryName().c_str();

    Services::getMqtt().send(type(), doc);
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

std::unique_ptr<BaseTask> AlertSensor::factory(
    const JsonObjectConst& parameters, Scheduler& scheduler) {
  auto alert_sensor =
      std::unique_ptr<AlertSensor>(new AlertSensor(parameters, scheduler));
  if (alert_sensor->isValid()) {
    return alert_sensor;
  } else {
    return std::unique_ptr<BaseTask>();
  }
}

const std::map<AlertSensor::TriggerType, const __FlashStringHelper*>
    AlertSensor::trigger_type_strings_{{TriggerType::kRising, F("rising")},
                                       {TriggerType::kFalling, F("falling")},
                                       {TriggerType::kEither, F("either")}};

}  // namespace tasks

}  // namespace bernd_box
