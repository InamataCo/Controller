#include "i2c_abstract_peripheral.h"

#include "managers/services.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace i2c_adapter {

I2CAbstractPeripheral::I2CAbstractPeripheral(const JsonObjectConst& parameter) {
  utils::UUID i2c_adapter_uuid(parameter[i2c_adapter_key_]);
  if (!i2c_adapter_uuid.isValid()) {
    setInvalid(i2c_adapter_key_error_);
    return;
  }

  std::shared_ptr<Peripheral> peripheral =
      Services::getPeripheralController().getPeripheral(i2c_adapter_uuid);

  // Since the UUID is specified externally, check the type
  if (peripheral->getType() == util::I2CAdapter::type() &&
      peripheral->isValid()) {
    i2c_adapter_ = std::static_pointer_cast<util::I2CAdapter>(peripheral);
  } else {
    setInvalid(invalidI2CAdapterError(i2c_adapter_uuid, peripheral->getType()));
    return;
  }
}

TwoWire* I2CAbstractPeripheral::getWire() { return i2c_adapter_->getWire(); }

bool I2CAbstractPeripheral::isDeviceConnected(uint16_t i2c_address) {
  getWire()->beginTransmission(i2c_address);
  byte error = Wire.endTransmission();
  return error == 0;
}

String I2CAbstractPeripheral::missingI2CDeviceError(int i2c_address) {
  String error(F("Cannot find device with I2C address: "));
  error += i2c_address;
  return error;
}

String I2CAbstractPeripheral::invalidI2CAdapterError(const utils::UUID& uuid,
                                                     const String& type) {
  String error = uuid.toString();
  error += F(" is not a valid ");
  error += type;
  return error;
}

const __FlashStringHelper* I2CAbstractPeripheral::i2c_address_key_ =
    FPSTR("i2c_address");
const __FlashStringHelper* I2CAbstractPeripheral::i2c_address_key_error_ =
    FPSTR("Missing property: i2c_address (uint16_t)");
const __FlashStringHelper* I2CAbstractPeripheral::i2c_adapter_key_ =
    FPSTR("i2c_adapter");
const __FlashStringHelper* I2CAbstractPeripheral::i2c_adapter_key_error_ =
    FPSTR("Missing property: i2c_adapter (uuid)");

}  // namespace i2c_adapter
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata
