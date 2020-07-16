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

ErrorResult PeripheralController::add(const JsonObjectConst& doc) {
  JsonVariantConst name = doc[F("name")];
  if (name.isNull() || !name.is<char*>()) {
    return ErrorResult(type(), F("Missing property: name (string)"));
  }

  // Check if element is present
  if (peripherals_.count(name) > 0) {
    return ErrorResult(type(), F("This name has already been added"));
  }

  // Not present, so try to create a new instance
  std::shared_ptr<peripheral::Peripheral> peripheral(
      peripheral_factory_.createPeripheral(doc));
  if (peripheral->isValid() == false) {
    return ErrorResult(type(), F("Failed creating peripheral"));
  }

  peripherals_.emplace(name.as<String>(), peripheral);

  return ErrorResult();
}

ErrorResult PeripheralController::remove(const JsonObjectConst& doc) {
  JsonVariantConst name = doc[F("name")];
  if (name.isNull() || !name.is<char*>()) {
    return ErrorResult(type(), F("Missing property: name (string)"));
  }

  auto iterator = peripherals_.find(name.as<String>());
  if (iterator != peripherals_.end()) {
    if (iterator->second.use_count() > 1) {
      return ErrorResult(type(), String(F("Object is still in use. Try again later for ")) +
                      name.as<String>());
    }
    peripherals_.erase(iterator);
  }

  return ErrorResult();
}

std::shared_ptr<peripheral::Peripheral> PeripheralController::getPeripheral(
    const String& name) {
  auto peripheral = peripherals_.find(name);
  if (peripheral != peripherals_.end()) {
    return peripheral->second;
  } else {
    return std::shared_ptr<peripheral::Peripheral>();
  }
}

void PeripheralController::handleCallback(const JsonObjectConst& message) {
  JsonVariantConst peripheral_commands = message[peripheral_command_name_];

  // Handle all add commands
  for (JsonVariantConst add_command :
       peripheral_commands[add_command_name_].as<JsonArrayConst>()) {
    ErrorResult error = add(add_command);
    if (error.is_error()) {
      server_.sendError(error, message[trace_id_name_].as<String>());
    }
  }

  // Handle all remove commands
  for (JsonVariantConst remove_command :
       peripheral_commands[remove_command_name_].as<JsonArrayConst>()) {
    ErrorResult error = remove(remove_command);
    if (error.is_error()) {
      server_.sendError(error, message[trace_id_name_].as<String>());
    }
  }
}

}  // namespace peripheral
}  // namespace bernd_box
