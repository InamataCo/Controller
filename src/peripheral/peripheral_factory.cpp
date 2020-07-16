#include "peripheral_factory.h"

namespace bernd_box {
namespace peripheral {

PeripheralFactory::PeripheralFactory(Server& server) : server_(server) {}

bool PeripheralFactory::registerFactory(const String& name, Callback factory) {
  if (getFactories().count(name) == 0) {
    getFactories().emplace(name, factory);
    return true;
  }
  return false;
}

std::shared_ptr<Peripheral> PeripheralFactory::createPeripheral(
    const JsonObjectConst& parameter) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  const JsonVariantConst type = parameter[F("type")];
  if (type.isNull() || !type.is<char*>()) {
    server_.sendError(who, "Missing property: type (string)");
    return std::make_shared<InvalidPeripheral>();
  }

  auto factory = getFactories().find(type);
  if (factory != getFactories().end()) {
    return factory->second(parameter);
  } else {
    return std::make_shared<InvalidPeripheral>();
  }
}

std::vector<String> PeripheralFactory::getFactoryNames() {
  std::vector<String> names;
  for (const auto& factory : getFactories()) {
    names.push_back(factory.first);
  }
  return names;
}

std::map<const String, PeripheralFactory::Callback>&
PeripheralFactory::getFactories() {
  static std::map<const String, Callback> factories;
  return factories;
}

}  // namespace peripheral
}  // namespace bernd_box
