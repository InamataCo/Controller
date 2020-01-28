#include "peripheryFactory.h"

#include "invalid_periphery.h"
#include "managers/services.h"
#include "peripheries/bh1750/bh1750_sensor.h"
#include "peripheries/dummy/dummyPeriphery.h"
#include "peripheries/util/I2CAdapter.h"

namespace bernd_box {
namespace periphery {

std::map<String, PeripheryFactory::Factory> PeripheryFactory::factories_;

bool PeripheryFactory::registerFactory(const String& name, Factory factory) {
  if (factories_.count(name) == 0) {
    factories_.emplace(name, factory);
    return true;
  }
  return false;
}

std::shared_ptr<Periphery> PeripheryFactory::createPeriphery(
    const JsonObjectConst& parameter) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  const JsonVariantConst type = parameter[F("type")];
  if (type.isNull() || !type.is<char*>()) {
    Services::getMqtt().sendError(who, "Missing property: name (string)");
    return std::make_shared<InvalidPeriphery>();
  }

  auto factory = factories_.find(type);
  if (factory != factories_.end()) {
    return factory->second(parameter);
  } else {
    return std::make_shared<InvalidPeriphery>();
  }
}

std::map<String, PeripheryFactory::Factory>& PeripheryFactory::getFactories() {
  return factories_;
}

}  // namespace periphery
}  // namespace bernd_box
