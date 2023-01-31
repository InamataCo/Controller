#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "managers/service_getters.h"
#include "peripheral/peripheral.h"
#include "peripheral/peripherals/uart/uart_adapter.h"
#include "utils/error_store.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace uart {

class UARTAdapter : public Peripheral {
 public:
  UARTAdapter(const ServiceGetters& services,
              const JsonObjectConst& parameters);
  virtual ~UARTAdapter();

  const String& getType() const final;
  static const String& type();

  /**
   * Get the configured Serial object
   * 
   * @return Pointer to active serial/UART interface
   */
  HardwareSerial* getSerial();

 private:
  static std::shared_ptr<Peripheral> factory(const ServiceGetters& services,
                                             const JsonObjectConst&);

#ifdef ESP32
  void setupESP32(int rx_pin, int tx_pin, const char* config, int baud_rate);
#else
  void setupESP8266(int rx_pin, int tx_pin, const char* config, int baud_rate);
#endif

  /**
   * Mark invalid as requested serial is taken
   *
   * Pass '0' for Serial/UART0, '1' for Serial1/UART1, '2' for Serial2/UART2
   *
   * @param serial_id Char value of taken UART (int --> char, 0 --> '0')
   */
  void setInvalidSerialTaken(const char serial_id);

  static bool registered_;

  static bool uart0_taken;
  static bool uart1_taken;
#ifdef ESP32
  static bool serial2_taken;
#endif

  std::shared_ptr<WebSocket> web_socket_;

  bool* taken_variable_ = nullptr;
  static const __FlashStringHelper* serial_taken_error_;
  static const __FlashStringHelper* invalid_pins_error_;

  std::shared_ptr<UARTAdapter> uart_adapter_;

  static const __FlashStringHelper* rx_key_;
  static const __FlashStringHelper* tx_key_;
  static const __FlashStringHelper* baud_rate_key_;
  static const __FlashStringHelper* config_key_;
  static const __FlashStringHelper* config_error_;
};

}  // namespace uart
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata
