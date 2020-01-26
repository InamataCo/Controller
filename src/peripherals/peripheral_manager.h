#ifndef PERIPHERAL_PERIPHERAL_MANAGER_H
#define PERIPHERAL_PERIPHERAL_MANAGER_H

#include <Arduino.h>

#include <map>
#include <memory>

#include "abstract_peripheral.h"
#include "abstract_peripheral_factory.h"
#include "managers/mqtt.h"

namespace bernd_box {

class PeripheralManager {
 public:
  PeripheralManager(
      std::shared_ptr<Mqtt> mqtt,
      std::unique_ptr<AbstractPeripheralFactory> peripheral_factory)
      : peripheral_factory_(std::move(peripheral_factory)),
        mqtt_(std::move(mqtt)) {}

  enum class Result { kSuccess, kFailure };

  Result parseObjectCommand(const JsonObjectConst& doc) {
    JsonVariantConst add_command = doc["add"];
    if (add_command.is<JsonObject>()) {
      add(add_command);
    }
    JsonVariantConst remove_command = doc["remove"];
    if (remove_command.is<JsonObject>()) {
      remove(remove_command);
    }
    return Result::kSuccess;
  }

  Result add(const JsonObjectConst& add_command) {
    const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

    // Check that the name is unique for all peripherals
    JsonVariantConst name = add_command[F("name")];
    if (name.isNull() || !name.is<char*>()) {
      mqtt_->sendError(who, F("Missing property: name (string)"));
      return Result::kFailure;
    }
    if (peripherals_.find(name) != peripherals_.end()) {
      mqtt_->sendError(who,
                       String(F("Name already exists: ")) + name.as<char*>());
      return Result::kFailure;
    }

    // Attempt to create a peripheral. On error, returns an invalid object
    std::unique_ptr<AbstractPeripheral> peripheral =
        peripheral_factory_->createPeripheral(add_command);
    if (peripheral->isValid()) {
      peripherals_.emplace(name.as<String>(), std::move(peripheral));
    } else {
      mqtt_->sendError(who, String(F("Error constructing peripheral: ")) +
                                peripheral->getError());
      return Result::kFailure;
    }

    return Result::kSuccess;
  }

  Result remove(const JsonObjectConst& remove_command) {
    return Result::kSuccess;
  }

  Result registerPeripheral(
      const String& name,
      std::unique_ptr<AbstractPeripheral> abstract_peripheral) {
    auto it = peripherals_.find(name);

    if (it == peripherals_.end()) {
      peripherals_[name] = std::move(abstract_peripheral);
      return Result::kSuccess;
    } else {
      return Result::kFailure;
    }
  }

 private:
  std::unique_ptr<AbstractPeripheralFactory> peripheral_factory_;
  std::map<String, std::unique_ptr<AbstractPeripheral>> peripherals_;

  std::shared_ptr<Mqtt> mqtt_;
};

}  // namespace bernd_box

#endif
