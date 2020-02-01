#ifndef BERND_BOX_PERIPHERY_FACTORY_H
#define BERND_BOX_PERIPHERY_FACTORY_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include <memory>
#include <vector>

#include "invalid_periphery.h"
#include "managers/mqtt.h"
#include "periphery.h"

namespace bernd_box {
namespace periphery {

class PeripheryFactory {
 public:
  using Callback =
      std::shared_ptr<Periphery> (*)(const JsonObjectConst& parameter);

  PeripheryFactory(Mqtt& mqtt);
  virtual ~PeripheryFactory() = default;

  static bool registerFactory(const String& name, Callback factory);

  std::shared_ptr<Periphery> createPeriphery(const JsonObjectConst& parameter);

  std::vector<String> getFactoryNames();

 private:
  Mqtt& mqtt_;
  static std::map<const String, Callback> factories_;
};

}  // namespace periphery
}  // namespace bernd_box

#endif
