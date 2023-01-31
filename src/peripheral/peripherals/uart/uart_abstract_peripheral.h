#pragma once

#include <ArduinoJson.h>

#include "peripheral/peripheral.h"
#include "peripheral/peripherals/uart/uart_adapter.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace uart {

/**
 * Interface for sensors to access peripherals over the UART/serial port
 */
class UARTAbstractPeripheral : public Peripheral {
 public:
  UARTAbstractPeripheral(const JsonObjectConst& parameters);

 protected:
  std::shared_ptr<UARTAdapter> uart_adapter_;

 private:
  static const __FlashStringHelper* uart_adapter_key_;
};

}  // namespace uart
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata
