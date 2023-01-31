#include "uart_abstract_peripheral.h"

#include "managers/services.h"
#include "utils/uuid.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace uart {

UARTAbstractPeripheral::UARTAbstractPeripheral(
    const JsonObjectConst& parameters) {
  utils::UUID uart_adapter_uuid(parameters[uart_adapter_key_]);
  if (!uart_adapter_uuid.isValid()) {
    setInvalid(ErrorStore::genMissingProperty(uart_adapter_key_,
                                              ErrorStore::KeyType::kUUID));
    return;
  }

  std::shared_ptr<Peripheral> peripheral =
      Services::getPeripheralController().getPeripheral(uart_adapter_uuid);
  if (peripheral->getType() == UARTAdapter::type() && peripheral->isValid()) {
    uart_adapter_ = std::static_pointer_cast<UARTAdapter>(peripheral);
  } else {
    setInvalid(
        ErrorStore::genNotAValid(uart_adapter_uuid, UARTAdapter::type()));
    return;
  }
}

const __FlashStringHelper* UARTAbstractPeripheral::uart_adapter_key_ =
    FPSTR("uart_adapter");

}  // namespace uart
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata
