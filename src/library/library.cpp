#include "library.h"

#include "config.h"
#include "periphery/periphery.h"
#include "periphery/peripheryFactory.h"

namespace bernd_box {
namespace library {

Library* Library::library_ = nullptr;

Library* Library::getLibrary(Mqtt& mqtt) {
  if (library_ == nullptr) {
    library_ = new Library(mqtt);
  }
  return library_;
}

Library::Library(Mqtt& mqtt) : mqtt_(mqtt) {}

Result Library::add(const JsonObjectConst& doc) {
  const char* who = __PRETTY_FUNCTION__;

  JsonVariantConst name = doc[F("name")];
  if (name.isNull() || !name.is<char*>()) {
    mqtt_.sendError(who, "Missing property: name (string)");
    return Result::kFailure;
  }

  // Check if element is present
  if (peripheries_.count(name) > 0) {
    mqtt_.sendError(who, "This name has already been added.");
    return Result::kFailure;
  }

  // Not present, so create new
  PeripheryFactory& factory = PeripheryFactory::getPeripheryFactory();
  std::shared_ptr<Periphery> periphery(
      factory.createPeriphery(*this, name.as<String>(), doc));
  peripheries_.insert(std::pair<String, std::shared_ptr<Periphery>>(
      name.as<String>(), periphery));

  return Result::kSuccess;
}

Result Library::remove(const JsonObjectConst& doc) {
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

Result Library::execute(const JsonObjectConst& doc) {
  const char* who = __PRETTY_FUNCTION__;

  JsonVariantConst name = doc[F("name")];
  if (name.isNull() || !name.is<char*>()) {
    mqtt_.sendError(who, "Missing property: name (string)");
    return Result::kFailure;
  }

  std::map<String, std::shared_ptr<Periphery>>::iterator iterator =
      peripheries_.find(name.as<String>());
  if (iterator == peripheries_.end()) {
    mqtt_.sendError(who, "No object found with name " + name.as<String>());
    return Result::kFailure;
  }
  Serial.println("Periphery = null ");
  Result result = iterator->second->executeTask(doc);
  if (result != Result::kSuccess) {
    mqtt_.sendError(who, "The chosen periphery of type " +
                             iterator->second->getType() +
                             " can not execute this task.");
  }
  return result;
}

std::shared_ptr<Periphery> Library::getPeriphery(String& name) {
  return peripheries_.find(name)->second;
}

Mqtt& Library::getMQTT() { return mqtt_; }

Result Library::handleCallback(char* topic, uint8_t* payload,
                               unsigned int length) {
  const char* who = __PRETTY_FUNCTION__;

  char* command = strrchr(topic, '/');
  command++;

  DynamicJsonDocument doc(BB_MQTT_JSON_PAYLOAD_SIZE);
  const DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    mqtt_.sendError(who, String(F("Deserialize failed: ")) + error.c_str());
    return Result::kFailure;
  }

  int action_topic_add = strncmp(command, BB_MQTT_TOPIC_ADD_SUFFIX,
                                 strlen(BB_MQTT_TOPIC_ADD_SUFFIX));
  if (action_topic_add == 0) return add(doc.as<JsonVariantConst>());
  int action_topic_remove = strncmp(command, BB_MQTT_TOPIC_REMOVE_PREFIX,
                                    strlen(BB_MQTT_TOPIC_REMOVE_PREFIX));
  if (action_topic_remove == 0) return remove(doc.as<JsonVariantConst>());
  int action_topic_execute = strncmp(command, BB_MQTT_TOPIC_EXECUTE_PREFIX,
                                     strlen(BB_MQTT_TOPIC_EXECUTE_PREFIX));
  if (action_topic_execute == 0) return execute(doc.as<JsonVariantConst>());
  mqtt_.sendError(
      who,
      String(F("Topic was neither add, nor remove, nor execute: ")) + command);
  return Result::kFailure;
}
}  // namespace library
}  // namespace bernd_box