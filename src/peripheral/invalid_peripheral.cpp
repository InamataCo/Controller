#include "invalid_peripheral.h"

#include "peripheral/peripheral_factory.h"

namespace bernd_box {
namespace peripheral {

InvalidPeripheral::InvalidPeripheral() { setInvalid(); }

InvalidPeripheral::InvalidPeripheral(const String& error) { setInvalid(error); }

const String& InvalidPeripheral::type() {
  static const String name{"InvalidPeripheral"};
  return name;
}

const String& InvalidPeripheral::getType() const { return type(); }

std::shared_ptr<Peripheral> InvalidPeripheral::factory(const ServiceGetters& services,
                                                       const JsonObjectConst&) {
  return std::make_shared<InvalidPeripheral>();
}

bool InvalidPeripheral::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

}  // namespace peripheral
}  // namespace bernd_box
