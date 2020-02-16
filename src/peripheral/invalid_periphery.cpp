#include "invalid_periphery.h"

namespace bernd_box {
namespace peripheral {

const String& InvalidPeriphery::type() {
  static const String name{"InvalidPeriphery"};
  return name;
}

const String& InvalidPeriphery::getType() { return type(); }

std::shared_ptr<Peripheral> InvalidPeriphery::factory(const JsonObjectConst&) {
  return std::make_shared<InvalidPeriphery>();
}

bool InvalidPeriphery::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

}  // namespace peripheral
}  // namespace bernd_box
