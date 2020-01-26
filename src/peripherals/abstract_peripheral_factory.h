#ifndef PERIPHERAL_ABSTRACT_PERIPHERAL_FACTORY
#define PERIPHERAL_ABSTRACT_PERIPHERAL_FACTORY

#include <ArduinoJson.h>

#include <memory>

#include "abstract_peripheral.h"

namespace bernd_box {

class AbstractPeripheralFactory {
 public:
  enum class Result {
    kSuccess,
  };

  virtual std::unique_ptr<AbstractPeripheral> createPeripheral(
      const JsonObjectConst& parameters) = 0;
};

}  // namespace bernd_box
#endif
