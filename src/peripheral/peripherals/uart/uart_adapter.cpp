#include "uart_adapter.h"

#include "peripheral/peripheral_factory.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace uart {

bool UARTAdapter::uart0_taken = false;
bool UARTAdapter::uart1_taken = false;
#ifdef ESP32
bool UARTAdapter::serial2_taken = false;
#endif

UARTAdapter::UARTAdapter(const ServiceGetters& services,
                         const JsonObjectConst& parameters) {
  web_socket_ = services.getWebSocket();
  if (web_socket_ == nullptr) {
    setInvalid(services.web_socket_nullptr_error_);
    return;
  }

  int rx_pin = -1;
  int tx_pin = -1;
  JsonVariantConst receive_pin = parameters[rx_key_];
  // Only assign value if it exists and is a Number (else 0 is returned)
  if (!receive_pin.isNull() && receive_pin.is<float>()) {
    rx_pin = receive_pin.as<int>();
  }
  JsonVariantConst transmit_pin = parameters[tx_key_];
  // Only assign value if it exists and is a Number (else 0 is returned)
  if (!transmit_pin.isNull() && transmit_pin.is<float>()) {
    tx_pin = transmit_pin.as<int>();
  }
  // 3 char encoding (default: 8N1)
  JsonVariantConst config_chars = parameters[config_key_];
  // baud rate (4800, 115200)
  JsonVariantConst baud_rate = parameters[baud_rate_key_];
  if (!baud_rate.is<float>()) {
    setInvalid(ErrorStore::genMissingProperty(baud_rate_key_,
                                              ErrorStore::KeyType::kString));
  }
#ifdef ESP32
  setupESP32(rx_pin, tx_pin, config_chars.as<const char*>(),
             baud_rate.as<int>());
#else
  setupESP8266(rx_pin, tx_pin, config_chars.as<const char*>(),
               baud_rate.as<float>());
#endif
}

UARTAdapter::~UARTAdapter() {
  if (taken_variable_) {
    *taken_variable_ = false;
  }
}

const String& UARTAdapter::getType() const { return type(); }

const String& UARTAdapter::type() {
  static const String name{"UARTAdapter"};
  return name;
}

HardwareSerial* UARTAdapter::getSerial() {
#ifdef ESP32
  return nullptr;
#else
  if (taken_variable_ == &uart0_taken) {
    return &Serial;
  } else if (taken_variable_ == &uart1_taken) {
    return &Serial1;
  } else {
    return nullptr;
  }
#endif
}

std::shared_ptr<Peripheral> UARTAdapter::factory(
    const ServiceGetters& services, const JsonObjectConst& parameters) {
  return std::make_shared<UARTAdapter>(services, parameters);
}

bool UARTAdapter::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

#ifdef ESP32
void UARTAdapter::setupESP32(int rx_pin, int tx_pin, const char* config_chars,
                             int baud_rate) {}
#endif

#ifdef ESP8266
void UARTAdapter::setupESP8266(int rx_pin, int tx_pin, const char* config_chars,
                               int baud_rate) {
  int config_bits = 0;
  if (!config_chars || strlen(config_chars) != 3) {
    config_bits = SERIAL_8N1;
  } else {
    const char data_bits = config_chars[0];
    switch (data_bits) {
      case '5':
        config_bits |= UART_NB_BIT_5;
        break;
      case '6':
        config_bits |= UART_NB_BIT_6;
        break;
      case '7':
        config_bits |= UART_NB_BIT_7;
        break;
      case '8':
        config_bits |= UART_NB_BIT_8;
        break;
      default:
        setInvalid(config_error_);
        return;
    }
    const char parity = config_chars[1];
    switch (parity) {
      case 'N':
        config_bits |= UART_PARITY_NONE;
        break;
      case 'E':
        config_bits |= UART_PARITY_EVEN;
        break;
      case 'O':
        config_bits |= UART_PARITY_ODD;
        break;
      default:
        setInvalid(config_error_);
        return;
    }
    const char stop_bits = config_chars[2];
    switch (stop_bits) {
      case '1':
        config_bits |= UART_NB_STOP_BIT_1;
        break;
      case '2':
        config_bits |= UART_NB_STOP_BIT_2;
        break;
      default:
        setInvalid(config_error_);
        return;
    }
  }

  if ((rx_pin == 3 && tx_pin == 1) || (rx_pin == 13 && tx_pin == 15)) {
    if (uart0_taken) {
      setInvalid(serial_taken_error_);
      return;
    }
    uart0_taken = true;
    taken_variable_ = &uart0_taken;
  } else if (tx_pin == 2) {
    if (uart1_taken) {
      setInvalid(serial_taken_error_);
      return;
    }
    uart1_taken = true;
    taken_variable_ = &uart1_taken;
  } else {
    setInvalid(invalid_pins_error_);
    return;
  }
  HardwareSerial* serial = getSerial();
  if (!serial) {
    setInvalid(serial_taken_error_);
    return;
  }
  serial->begin(baud_rate, static_cast<SerialConfig>(config_bits));
  if (rx_pin == 13 && tx_pin == 15) {
    serial->pins(tx_pin, rx_pin);
  }
}
#endif

const __FlashStringHelper* UARTAdapter::serial_taken_error_ =
    FPSTR("UART taken/nullptr");
const __FlashStringHelper* UARTAdapter::invalid_pins_error_ =
    FPSTR("Invalid pins");

const __FlashStringHelper* UARTAdapter::rx_key_ = FPSTR("rx");
const __FlashStringHelper* UARTAdapter::tx_key_ = FPSTR("tx");
const __FlashStringHelper* UARTAdapter::baud_rate_key_ = FPSTR("baud_rate");
const __FlashStringHelper* UARTAdapter::config_key_ = FPSTR("config");
const __FlashStringHelper* UARTAdapter::config_error_ = FPSTR("Invalid config");

}  // namespace uart
}  // namespace peripherals
}  // namespace peripheral

}  // namespace inamata
