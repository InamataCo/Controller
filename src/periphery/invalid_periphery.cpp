#include "invalid_periphery.h"

namespace bernd_box {
namespace periphery {

bool InvalidPeriphery::registered_ =
    PeripheryFactory::registerFactory(type(), factory);

const __FlashStringHelper* InvalidPeriphery::type() {
  return F("InvalidPeriphery");
}

const __FlashStringHelper* InvalidPeriphery::getType() { return type(); }

std::shared_ptr<Periphery> InvalidPeriphery::factory(const JsonObjectConst&) {
  return std::make_shared<InvalidPeriphery>();
}

}  // namespace periphery
}  // namespace bernd_box
