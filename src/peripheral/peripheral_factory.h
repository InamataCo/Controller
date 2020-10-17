#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include <map>
#include <memory>
#include <vector>

#include "managers/server.h"
#include "peripheral/invalid_peripheral.h"
#include "peripheral/peripheral.h"

namespace bernd_box {
namespace peripheral {

class PeripheralFactory {
 public:
  using Callback =
      std::shared_ptr<Peripheral> (*)(const JsonObjectConst& parameter);

  PeripheralFactory(Server& server);
  virtual ~PeripheralFactory() = default;

  static bool registerFactory(const String& name, Callback factory);

  std::shared_ptr<Peripheral> createPeripheral(
      const JsonObjectConst& parameter);

  std::vector<String> getFactoryNames();

 private:
  static std::map<const String, Callback>& getFactories();

  static String unknownTypeError(const String& type);

  Server& server_;

  static const __FlashStringHelper* type_key_;
  static const __FlashStringHelper* type_key_error_;
};

}  // namespace peripheral
}  // namespace bernd_box