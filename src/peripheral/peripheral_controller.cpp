#include "peripheral_controller.h"

namespace bernd_box {
namespace peripheral {

PeripheralController::PeripheralController(
    Server& server, peripheral::PeripheralFactory& peripheral_factory)
    : server_(server), peripheral_factory_(peripheral_factory) {}

const String& PeripheralController::type() {
  static const String name{"PeripheralController"};
  return name;
}

void PeripheralController::handleCallback(const JsonObjectConst& message) {
  // Check if any peripheral commands have to be processed
  JsonVariantConst peripheral_commands = message[peripheral_command_key_];
  if (!peripheral_commands) {
    return;
  }

  // Init the result doc with type and the request ID
  DynamicJsonDocument result_doc(BB_JSON_PAYLOAD_SIZE);
  result_doc[Server::type_key_] = Server::result_type_;
  result_doc[Server::request_id_key_] = message[Server::request_id_key_];
  JsonObject peripheral_results =
      result_doc.createNestedObject(peripheral_command_key_);

  // Add a peripheral for each command and store the result
  JsonArrayConst add_commands =
      peripheral_commands[add_command_key_].as<JsonArrayConst>();
  if (add_commands) {
    JsonArray add_results =
        peripheral_results.createNestedArray(add_command_key_);
    for (JsonVariantConst add_command : add_commands) {
      ErrorResult error = add(add_command);
      addResultEntry(add_command["uuid"], error, add_results);
    }
  }

  // Remove a peripheral for each command and store the result
  JsonArrayConst remove_commands =
      peripheral_commands[remove_command_key_].as<JsonArrayConst>();
  if (remove_commands) {
    JsonArray remove_results =
        peripheral_results.createNestedArray(remove_command_key_);
    for (JsonVariantConst remove_command : remove_commands) {
      ErrorResult error = remove(remove_command);
      addResultEntry(remove_command["uuid"], error, remove_results);
    }
  }

  // Send the command results
  server_.sendResults(result_doc.as<JsonObject>());
}

ErrorResult PeripheralController::add(const JsonObjectConst& doc) {
  utils::UUID uuid(doc[uuid_key_]);
  if (!uuid.isValid()) {
    return ErrorResult(type(), uuid_key_error_);
  }

  // Check if element is present
  if (peripherals_.count(uuid) > 0) {
    return ErrorResult(type(), F("This peripheral has already been added"));
  }

  // Not present, so try to create a new instance
  std::shared_ptr<peripheral::Peripheral> peripheral(
      peripheral_factory_.createPeripheral(doc));
  if (peripheral) {
    if (!peripheral->isValid()) {
      return peripheral->getError();
    }
  } else {
    return ErrorResult(type(), F("Error calling peripheral factory"));
  }

  peripherals_.emplace(uuid, peripheral);

  return ErrorResult();
}

ErrorResult PeripheralController::remove(const JsonObjectConst& doc) {
  utils::UUID uuid(doc[uuid_key_]);
  if (!uuid.isValid()) {
    return ErrorResult(type(), uuid_key_error_);
  }

  auto iterator = peripherals_.find(uuid);
  if (iterator != peripherals_.end()) {
    if (iterator->second.use_count() > 1) {
      return ErrorResult(type(), String(F("Peripheral still in use")));
    }
    peripherals_.erase(iterator);
  }

  return ErrorResult();
}

std::shared_ptr<peripheral::Peripheral> PeripheralController::getPeripheral(
    const utils::UUID& uuid) {
  auto peripheral = peripherals_.find(uuid);
  if (peripheral != peripherals_.end()) {
    return peripheral->second;
  } else {
    return std::shared_ptr<peripheral::Peripheral>();
  }
}

void PeripheralController::addResultEntry(const JsonVariantConst& uuid,
                                          const ErrorResult& error,
                                          const JsonArray& results) {
  JsonObject result = results.createNestedObject();

  // Save whether the peripheral could be created or the reason for failing
  if (error.isError()) {
    result[uuid_key_] = uuid;
    result["status"] = "fail";
    result["detail"] = error.detail_;
  } else {
    result[uuid_key_] = uuid;
    result["status"] = "success";
  }
}

const __FlashStringHelper* PeripheralController::peripheral_command_key_ =
    F("peripheral");
const __FlashStringHelper* PeripheralController::uuid_key_ = F("uuid");
const __FlashStringHelper* PeripheralController::uuid_key_error_ =
    F("Missing property: uuid (uuid)");
const __FlashStringHelper* PeripheralController::add_command_key_ = F("add");
const __FlashStringHelper* PeripheralController::remove_command_key_ =
    F("remove");

}  // namespace peripheral
}  // namespace bernd_box
