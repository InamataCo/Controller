#include "invalid_periphery.h"

namespace bernd_box {
namespace periphery {

const String& InvalidPeriphery::type() {
  static const String name{"InvalidPeriphery"};
  return name;
}

const String& InvalidPeriphery::getType() { return type(); }

std::shared_ptr<Periphery> InvalidPeriphery::factory(const JsonObjectConst&) {
  return std::make_shared<InvalidPeriphery>();
}

bool InvalidPeriphery::registered_ =
    PeripheryFactory::registerFactory(type(), factory);

}  // namespace periphery
}  // namespace bernd_box
