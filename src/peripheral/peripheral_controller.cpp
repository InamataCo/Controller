#include "peripheral_controller.h"

#include "managers/services.h"

namespace bernd_box {
namespace peripheral {

PeripheralController::PeripheralController(Mqtt& mqtt, peripheral::PeripheralFactory& periphery_factory)
    : mqtt_(mqtt), periphery_factory_(periphery_factory) {}

Result PeripheralController::add(const JsonObjectConst& doc) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  JsonVariantConst name = doc[F("name")];
  if (name.isNull() || !name.is<char*>()) {
    mqtt_.sendError(who, F("Missing property: name (string)"));
    return Result::kFailure;
  }

  // Check if element is present
  if (peripheries_.count(name) > 0) {
    mqtt_.sendError(who, F("This name has already been added."));
    return Result::kFailure;
  }

  // Not present, so create new
  std::shared_ptr<peripheral::Peripheral> periphery(
      periphery_factory_.createPeriphery(doc));
  if (periphery->isValid() == false) {
    return Result::kFailure;
  }

  peripheries_.emplace(name.as<String>(), periphery);

  return Result::kSuccess;
}

Result PeripheralController::remove(const JsonObjectConst& doc) {
  const char* who = __PRETTY_FUNCTION__;

  JsonVariantConst name = doc[F("name")];
  if (name.isNull() || !name.is<char*>()) {
    mqtt_.sendError(who, String(F("Missing property: name (string)")));
    return Result::kFailure;
  }

  auto iterator = peripheries_.find(name.as<String>());
  if (iterator != peripheries_.end()) {
    if (iterator->second.use_count() > 1) {
      mqtt_.sendError(
          who, String(F("Object is still in use. Try again later for ")) +
                   name.as<String>());
      return Result::kFailure;
    }
    peripheries_.erase(iterator);
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

  auto iterator = peripheries_.find(name.as<String>());
  if (iterator == peripheries_.end()) {
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

  peripheral::PeripheryTask& peripheryTask =
      task_factory.createTask(iterator->second, parameter);
  Services::getScheduler().addTask(peripheryTask);
  peripheryTask.enable();

  JsonVariantConst response_code = doc[F(PERIPHERY_TASK_RESPONSE_CODE_NODE)];
  if (!response_code.isNull() && response_code.is<char*>()) {
    DynamicJsonDocument resultDoc(BB_MQTT_JSON_PAYLOAD_SIZE);
    resultDoc[PERIPHERY_TASK_RESPONSE_CODE_NODE] = response_code.as<String>();
    resultDoc[PERIPHERY_TASK_TASK_ID_NODE] = peripheryTask.getId();
    mqtt_.send(who, resultDoc);
  }

  return Result::kSuccess;
} */

std::shared_ptr<peripheral::Peripheral> PeripheralController::getPeriphery(
    const String& name) {
  auto periphery = peripheries_.find(name);
  if (periphery != peripheries_.end()) {
    return periphery->second;
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
