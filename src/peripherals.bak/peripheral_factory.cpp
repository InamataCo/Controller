#include "peripheral_factory.h"

namespace bernd_box {

std::unique_ptr<AbstractPeripheral> PeripheralFactory::createPeripheral(
    const JsonObjectConst& parameters) {
  return std::unique_ptr<InvalidPeripheral>(new InvalidPeripheral());
}

}  // namespace bernd_box
