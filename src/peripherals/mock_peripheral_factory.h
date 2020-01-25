#ifndef PERIPHERAL_MOCK_PERIPHERAL_FACTORY
#define PERIPHERAL_MOCK_PERIPHERAL_FACTORY

#include "abstract_peripheral_factory.h"

class MockPeripheralFactory : public AbstractPeripheralFactory {
  virtual std::unique_ptr<AbstractPeripheral> createPeripheral(
      const JsonObjectConst& parameters) = 0;
};

#endif
