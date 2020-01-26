#include "I2CAbstractPeriphery.h"

#include "managers/services.h"

namespace bernd_box {
namespace periphery {
namespace peripheries {

I2CAbstractPeriphery::I2CAbstractPeriphery(const JsonObjectConst& parameter) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  JsonVariantConst i2c_adapter_name =
      parameter[F(PARAM_I2C_ABSTRACT_PERIPHERY_ADAPTER_NAME)];
  if (i2c_adapter_name.isNull() || !i2c_adapter_name.is<char*>()) {
    Services::getMQTT().sendError(
        who, F("Missing property: " PARAM_I2C_ABSTRACT_PERIPHERY_ADAPTER_NAME
               " (string)"));
    setInvalid();
    return;
  }

  i2CAdapter_ = std::dynamic_pointer_cast<I2CAdapter>(
      Services::getLibrary().getPeriphery(i2c_adapter_name.as<String>()));
  if (i2CAdapter_ == nullptr) {
    Services::getMQTT().sendError(
        who,
        i2c_adapter_name.as<String>() +
            String(F(" is not registered or not a valid " TYPE_I2CADAPTER)));
    setInvalid();
    return;
  }
}

TwoWire* I2CAbstractPeriphery::getWire() { return i2CAdapter_->getWire(); }

}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box