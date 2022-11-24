#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include <map>
#include <memory>
#include <vector>

#include "managers/service_getters.h"
#include "peripheral/peripheral.h"

namespace inamata {
namespace peripheral {

class PeripheralFactory {
 public:
  using Callback = std::shared_ptr<Peripheral> (*)(
      const ServiceGetters& services, const JsonObjectConst& parameter);

  PeripheralFactory() = default;
  virtual ~PeripheralFactory() = default;

  static bool registerFactory(const String& name, Callback factory);

  std::shared_ptr<Peripheral> createPeripheral(
      const ServiceGetters& services, const JsonObjectConst& parameter);

  std::vector<String> getFactoryNames();

 private:
  static std::map<const String, Callback>& getFactories();

  static String unknownTypeError(const String& type);

  static const __FlashStringHelper* type_key_;
  static const __FlashStringHelper* type_key_error_;
};

}  // namespace peripheral
}  // namespace inamata