#include "i2c_abstract_peripheral.h"

#include "managers/services.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {

I2CAbstractPeripheral::I2CAbstractPeripheral(const JsonObjectConst& parameter) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  JsonVariantConst i2c_adapter_name = parameter[F("adapter")];
  if (i2c_adapter_name.isNull() || !i2c_adapter_name.is<char*>()) {
    Services::getMqtt().sendError(who, F("Missing property: adapter (string)"));
    setInvalid();
    return;
  }

  const String name = i2c_adapter_name.as<String>();
  std::shared_ptr<Peripheral> peripheral =
      Services::getPeripheralController().getPeripheral(name);

  // Since the name is specified externally, check the type
  if (peripheral->getType() == peripherals::util::I2CAdapter::type() &&
      peripheral->isValid()) {
    i2c_adapter_ =
        std::static_pointer_cast<peripherals::util::I2CAdapter>(peripheral);
  } else {
    Services::getMqtt().sendError(
        who,
        name + F(" is not a valid ") + peripherals::util::I2CAdapter::type());
    setInvalid();
    return;
  }
}

TwoWire* I2CAbstractPeripheral::getWire() { return i2c_adapter_->getWire(); }

}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
