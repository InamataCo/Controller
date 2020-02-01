#ifndef BERND_BOX_PERIPHERY_PERIPHERIES_I2CABSTRACTPERIPHERY_H
#define BERND_BOX_PERIPHERY_PERIPHERIES_I2CABSTRACTPERIPHERY_H

#define PARAM_I2C_ABSTRACT_PERIPHERY_ADAPTER_NAME "adapter"

#include <Wire.h>
#include "periphery/abstract_periphery.h"
#include "periphery/peripheries/util/i2c_adapter.h"

namespace bernd_box {
namespace periphery {
namespace peripheries {

using namespace util;

class I2CAbstractPeriphery : public AbstractPeriphery {
 public:
  I2CAbstractPeriphery(const JsonObjectConst& parameter);
  virtual ~I2CAbstractPeriphery() = default;

 protected:
  TwoWire* getWire();

 private:
  std::shared_ptr<I2CAdapter> i2c_adapter_;
};

}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box

#endif
