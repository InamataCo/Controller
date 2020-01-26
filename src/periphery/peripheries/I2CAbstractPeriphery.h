#ifndef BERND_BOX_PERIPHERY_PERIPHERIES_I2CABSTRACTPERIPHERY_H
#define BERND_BOX_PERIPHERY_PERIPHERIES_I2CABSTRACTPERIPHERY_H

#define PARAM_I2C_ABSTRACT_PERIPHERY_ADAPTER_NAME "adapter"

#include <Wire.h>
#include "periphery/abstractPeriphery.h"
#include "periphery/peripheries/util/I2CAdapter.h"

namespace bernd_box {
namespace periphery {
namespace peripheries {

using namespace util;

class I2CAbstractPeriphery : public AbstractPeriphery {
 private:
  std::shared_ptr<I2CAdapter> i2CAdapter_;

 protected:
  TwoWire* getWire();

 public:
  I2CAbstractPeriphery(const JsonObjectConst& parameter);
  virtual ~I2CAbstractPeriphery() = default;
};

}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box

#endif