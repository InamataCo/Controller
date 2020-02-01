#ifndef PERIPHERAL_PERIPHERAL_FACTORY
#define PERIPHERAL_PERIPHERAL_FACTORY

#include <memory>

#include "abstract_peripheral_factory.h"
#include "invalid_peripheral.h"

namespace bernd_box {

class PeripheralFactory : public AbstractPeripheralFactory {
  std::unique_ptr<AbstractPeripheral> createPeripheral(
      const JsonObjectConst& parameters) final;
};

};  // namespace bernd_box

#endif
