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
    Services::getMqtt().sendError(
        who, F("Missing property: " PARAM_I2C_ABSTRACT_PERIPHERY_ADAPTER_NAME
               " (string)"));
    setInvalid();
    return;
  }

  const String name = i2c_adapter_name.as<String>();
  std::shared_ptr<Periphery> periphery =
      Services::getLibrary().getPeriphery(name);

  // Since the name is specified externally (MQTT message), check type
  if (periphery->getType() == I2CAdapter::type() && periphery->isValid()) {
    i2c_adapter_ = std::static_pointer_cast<I2CAdapter>(periphery);
  } else {
    Services::getMqtt().sendError(
        who, name + F(" is not a valid ") + I2CAdapter::type());
    setInvalid();
    return;
  }
}

TwoWire* I2CAbstractPeriphery::getWire() { return i2c_adapter_->getWire(); }

}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box
