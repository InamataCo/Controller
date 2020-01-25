#include "library.h"

#include "periphery/periphery.h"
#include "periphery/peripheryFactory.h"

namespace bernd_box {
namespace library {

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
  Periphery& periphery = factory.createPeriphery(doc);
  peripheries_.insert(std::pair<String, Periphery&>(name, periphery));

  return Result::kSuccess;
}

Result Library::remove(const JsonObjectConst& doc) {
  const char* who = __PRETTY_FUNCTION__;

  JsonVariantConst name = doc[F("name")];
  if (name.isNull() || !name.is<char*>()) {
    mqtt_.sendError(who, "Missing property: name (string)");
    return Result::kFailure;
  }

  std::map<String, Periphery&>::iterator iterator = peripheries_.find(name);
  if (iterator != peripheries_.end()) peripheries_.erase(iterator);

  return Result::kSuccess;
}

Result Library::execute(const JsonObjectConst& doc) {
  const char* who = __PRETTY_FUNCTION__;

  JsonVariantConst name = doc[F("name")];
  if (name.isNull() || !name.is<char*>()) {
    mqtt_.sendError(who, "Missing property: name (string)");
    return Result::kFailure;
  }

  std::map<String, Periphery&>::iterator iterator = peripheries_.find(name);
  Periphery& periphery = iterator->second;
  Result result = periphery.executeTask(doc);
  if (result != Result::kSuccess) {
    mqtt_.sendError(who, "The chosen periphery of type " + periphery.getType() +
                             " can not execute this task.");
  }
  return result;
}

Periphery& Library::getPeriphery(String& name) {
  std::map<String, Periphery&>::iterator iterator = peripheries_.find(name);
  return iterator->second;
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
}

}  // namespace library
}  // namespace bernd_box