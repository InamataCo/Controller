#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include <memory>
#include <vector>

#include "managers/mqtt.h"
#include "peripheral.h"

namespace bernd_box {
namespace peripheral {

class PeripheralFactory {
 public:
  using Callback =
      std::shared_ptr<Peripheral> (*)(const JsonObjectConst& parameter);

  PeripheralFactory(Mqtt& mqtt);
  virtual ~PeripheralFactory() = default;

  static bool registerFactory(const String& name, Callback factory);

  std::shared_ptr<Peripheral> createPeriphery(const JsonObjectConst& parameter);

  std::vector<String> getFactoryNames();

 private:
  Mqtt& mqtt_;
  static std::map<const String, Callback> factories_;
};

}  // namespace peripheral
}  // namespace bernd_box