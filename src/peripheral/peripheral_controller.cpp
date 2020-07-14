#include "peripheral_controller.h"

namespace bernd_box {
namespace peripheral {

PeripheralController::PeripheralController(Mqtt& mqtt, peripheral::PeripheralFactory& peripheral_factory)
    : mqtt_(mqtt), peripheral_factory_(peripheral_factory) {}

Result PeripheralController::add(const JsonObjectConst& doc) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  JsonVariantConst name = doc[F("name")];
  if (name.isNull() || !name.is<char*>()) {
    mqtt_.sendError(who, F("Missing property: name (string)"));
    return Result::kFailure;
  }

  // Check if element is present
  if (peripherals_.count(name) > 0) {
    mqtt_.sendError(who, F("This name has already been added."));
    return Result::kFailure;
  }

  // Not present, so create new
  std::shared_ptr<peripheral::Peripheral> peripheral(
      peripheral_factory_.createPeripheral(doc));
  if (peripheral->isValid() == false) {
    return Result::kFailure;
  }

  peripherals_.emplace(name.as<String>(), peripheral);

  return Result::kSuccess;
}

Result PeripheralController::remove(const JsonObjectConst& doc) {
  const char* who = __PRETTY_FUNCTION__;

  JsonVariantConst name = doc[F("name")];
  if (name.isNull() || !name.is<char*>()) {
    mqtt_.sendError(who, String(F("Missing property: name (string)")));
    return Result::kFailure;
  }

  auto iterator = peripherals_.find(name.as<String>());
  if (iterator != peripherals_.end()) {
    if (iterator->second.use_count() > 1) {
      mqtt_.sendError(
          who, String(F("Object is still in use. Try again later for ")) +
                   name.as<String>());
      return Result::kFailure;
    }
    peripherals_.erase(iterator);
  }

  return Result::kSuccess;
}

/* Result Library::execute(const JsonObjectConst& doc) {
  const char* who = __PRETTY_FUNCTION__;

  JsonVariantConst name = doc[F(PERIPHERY_TASK_NAME_NODE)];
  if (name.isNull() || !name.is<char*>()) {
    mqtt_.sendError(who, String(F("Missing property: " PERIPHERY_TASK_NAME_NODE
                                  " (string)")));
    return Result::kFailure;
  }

  auto iterator = peripherals_.find(name.as<String>());
  if (iterator == peripherals_.end()) {
    mqtt_.sendError(
        who, String(F("No object found with name ")) + name.as<String>());
    return Result::kFailure;
  }
  peripheral::TaskFactory& task_factory = iterator->second->getTaskFactory(doc);

  JsonVariantConst parameter = doc[F(PERIPHERY_TASK_PARAMETER_NODE)];
  if (parameter.isNull()) {
    mqtt_.sendError(who,
                    String(F("Missing property: " PERIPHERY_TASK_PARAMETER_NODE
                             " (may be empty)")));
    return Result::kFailure;
  }

  peripheral::PeripheralTask& peripheralTask =
      task_factory.createTask(iterator->second, parameter);
  Services::getScheduler().addTask(peripheralTask);
  peripheralTask.enable();

  JsonVariantConst response_code = doc[F(PERIPHERY_TASK_RESPONSE_CODE_NODE)];
  if (!response_code.isNull() && response_code.is<char*>()) {
    DynamicJsonDocument resultDoc(BB_MQTT_JSON_PAYLOAD_SIZE);
    resultDoc[PERIPHERY_TASK_RESPONSE_CODE_NODE] = response_code.as<String>();
    resultDoc[PERIPHERY_TASK_TASK_ID_NODE] = peripheralTask.getId();
    mqtt_.send(who, resultDoc);
  }

  return Result::kSuccess;
} */

std::shared_ptr<peripheral::Peripheral> PeripheralController::getPeripheral(
    const String& name) {
  auto peripheral = peripherals_.find(name);
  if (peripheral != peripherals_.end()) {
    return peripheral->second;
  } else {
    return std::shared_ptr<peripheral::Peripheral>();
  }
}

void PeripheralController::handleCallback(char* topic, uint8_t* payload,
                             unsigned int length) {
  const char* who = __PRETTY_FUNCTION__;

  char* command = strrchr(topic, '/');
  command++;

  DynamicJsonDocument doc(BB_MQTT_JSON_PAYLOAD_SIZE);
  const DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    mqtt_.sendError(who, String(F("Deserialize failed: ")) + error.c_str());
    return;
  }

  int action_topic_add = strncmp(command, BB_MQTT_TOPIC_ADD_SUFFIX,
                                 strlen(BB_MQTT_TOPIC_ADD_SUFFIX));
  if (action_topic_add == 0) {
    add(doc.as<JsonVariantConst>());
    return;
  }
  int action_topic_remove = strncmp(command, BB_MQTT_TOPIC_REMOVE_PREFIX,
                                    strlen(BB_MQTT_TOPIC_REMOVE_PREFIX));
  if (action_topic_remove == 0) {
    remove(doc.as<JsonVariantConst>());
    return;
  }
  mqtt_.sendError(
      who, String(F("Unknown action [add, remove, execute]: ")) + command);
  return;
}

}  // namespace library
}  // namespace bernd_box
