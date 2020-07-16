#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include <memory>
#include <vector>
#include <map>

#include "managers/server.h"
#include "peripheral/peripheral.h"
#include "peripheral/invalid_peripheral.h"

namespace bernd_box {
namespace peripheral {

class PeripheralFactory {
 public:
  using Callback =
      std::shared_ptr<Peripheral> (*)(const JsonObjectConst& parameter);

  PeripheralFactory(Server& server);
  virtual ~PeripheralFactory() = default;

  static bool registerFactory(const String& name, Callback factory);

  std::shared_ptr<Peripheral> createPeripheral(const JsonObjectConst& parameter);

  std::vector<String> getFactoryNames();

 private:
  static std::map<const String, Callback>& getFactories();

  Server& server_;
};

}  // namespace peripheral
}  // namespace bernd_box