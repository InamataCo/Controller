#include "pump.h"

namespace bernd_box {

namespace tasks {

using namespace std::placeholders;

Pump::Pump(Scheduler* scheduler, Io& io, Mqtt& mqtt)
    : Task(scheduler), io_(io), mqtt_(mqtt) {
  setIterations(TASK_FOREVER);
  setInterval(TASK_HOUR);  // All logic is in OnEnable and OnDisable
  mqtt_.addAction(String("pump"),
                  std::bind(&Pump::MqttCallback, this, _1, _2, _3));
  // By default set 60 seconds pump duration
  setDuration(std::chrono::seconds(60));
}
Pump::~Pump() {}

void Pump::setDuration(const std::chrono::milliseconds duration) {
  setTimeout(duration.count());
}

const std::chrono::milliseconds Pump::getDuration() {
  return std::chrono::milliseconds(getTimeout());
}

void Pump::MqttCallback(char* topic, uint8_t* payload, unsigned int length) {
  const char* who = __PRETTY_FUNCTION__;

  // Try to deserialize the message
  StaticJsonDocument<BB_MQTT_JSON_PAYLOAD_SIZE> doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    mqtt_.sendError(who,
                    String(F("deserialize JSON failed: ")) + error.c_str());
    return;
  }

  // Execute action and its parameters. Expect atleast one valid parameter
  bool valid_action = false;

  // Set the duration to keep the pump on. 0 for forever
  if (doc.containsKey(F("duration_s"))) {
    std::chrono::seconds duration(doc[F("duration_s")]);
    if (duration < std::chrono::seconds(0)) {
      mqtt_.sendError(who, F("Invalid duration less than 0."));
      return;
    }
    setDuration(duration);
    valid_action = true;
  }

  // Enable or disable the pump
  if (doc.containsKey(F("enable"))) {
    if (doc[F("enable")]) {
      this->enable();
    } else {
      this->disable();
    }
    valid_action = true;
  }

  // If no valid action was performed, send an error
  if (!valid_action) {
    mqtt_.sendError(who, F("No valid keys found for action"));
  }
}

bool Pump::OnEnable() {
  io_.setPinState(io_.atx_power_pin_, true);
  io_.setPinState(io_.pump_pin_, true);

  DynamicJsonDocument doc(BB_MQTT_JSON_PAYLOAD_SIZE);
  doc["enable"] = true;
  doc["duration_s"] =
      std::chrono::duration_cast<std::chrono::seconds>(getDuration()).count();
  mqtt_.send(String(F("pump")), doc);
  start_time_ = std::chrono::milliseconds(millis());

  return true;
}

bool Pump::Callback() { return true; }

void Pump::OnDisable() {
  const auto duration = std::chrono::milliseconds(millis()) - start_time_;

  DynamicJsonDocument doc(BB_MQTT_JSON_PAYLOAD_SIZE);
  doc["enable"] = false;
  doc["planned_duration_s"] =
      std::chrono::duration_cast<std::chrono::seconds>(getDuration()).count();
  doc["actual_duration_s"] =
      std::chrono::duration_cast<std::chrono::seconds>(duration).count();
  mqtt_.send(String(F("pump")), doc);

  setTimeout(TASK_NOTIMEOUT);

  io_.setPinState(io_.atx_power_pin_, false);
  io_.setPinState(io_.pump_pin_, false);
}

}  // namespace tasks

}  // namespace bernd_box
