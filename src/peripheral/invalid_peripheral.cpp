#include "invalid_peripheral.h"

namespace bernd_box {
namespace peripheral {

const String& InvalidPeripheral::type() {
  static const String name{"InvalidPeripheral"};
  return name;
}

const String& InvalidPeripheral::getType() { return type(); }

std::shared_ptr<Peripheral> InvalidPeripheral::factory(const JsonObjectConst&) {
  return std::make_shared<InvalidPeripheral>();
}

bool InvalidPeripheral::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

}  // namespace peripheral
}  // namespace bernd_box
