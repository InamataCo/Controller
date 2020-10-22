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

void PeripheralController::handleCallback(const JsonObjectConst& message) {
  JsonVariantConst peripheral_commands = message[peripheral_command_key_];

  // Handle all add commands
  for (JsonVariantConst add_command :
       peripheral_commands[add_command_key_].as<JsonArrayConst>()) {
    ErrorResult error = add(add_command);
    if (error.is_error()) {
      server_.sendError(error, message[request_id_key_].as<String>());
    }
  }

  // Handle all remove commands
  for (JsonVariantConst remove_command :
       peripheral_commands[remove_command_key_].as<JsonArrayConst>()) {
    ErrorResult error = remove(remove_command);
    if (error.is_error()) {
      server_.sendError(error, message[request_id_key_].as<String>());
    }
  }
}

}  // namespace peripheral
}  // namespace bernd_box
