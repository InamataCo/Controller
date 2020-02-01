#include "periphery_factory.h"
#include "invalid_periphery.h"

namespace bernd_box {
namespace periphery {

std::map<const String, PeripheryFactory::Callback> PeripheryFactory::factories_;

PeripheryFactory::PeripheryFactory(Mqtt& mqtt) : mqtt_(mqtt) {}

bool PeripheryFactory::registerFactory(const String& name, Callback factory) {
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
    mqtt_.sendError(who, "Missing property: type (string)");
    return std::make_shared<InvalidPeriphery>();
  }

  auto factory = factories_.find(type);
  if (factory != factories_.end()) {
    return factory->second(parameter);
  } else {
    return std::make_shared<InvalidPeriphery>();
  }
}

std::vector<String> PeripheryFactory::getFactoryNames() {
  std::vector<String> names;
  for(const auto& factory : factories_) {
    names.push_back(factory.first);
  }
  return names;
}

}  // namespace periphery
}  // namespace bernd_box
