#ifdef ESP32
#include "pwm.h"

#include "peripheral/peripheral_factory.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace pwm {

Pwm::Pwm(const ServiceGetters& services, const JsonObjectConst& parameters) {
  web_socket_ = services.getWebSocket();
  if (web_socket_ == nullptr) {
    setInvalid(services.web_socket_nullptr_error_);
    return;
  }

  int pin = toPin(parameters[pin_key_]);
  if (pin < 0) {
    setInvalid(pin_key_error_);
    return;
  }

  data_point_type_ =
      utils::UUID(parameters[utils::ValueUnit::data_point_type_key]);
  if (!data_point_type_.isValid()) {
    setInvalid(utils::ValueUnit::data_point_type_key_error);
    return;
  }

  bool error = setup(pin);
  if (error) {
    setInvalid(no_channels_available_error_);
    return;
  }
}

Pwm::~Pwm() { freeResources(); }

const String& Pwm::getType() const { return type(); }

const String& Pwm::type() {
  static const String name{"PWM"};
  return name;
}

void Pwm::setValue(utils::ValueUnit value_unit) {
  if (value_unit.data_point_type != data_point_type_) {
    web_socket_->sendError(type(),
                           value_unit.sourceUnitError(data_point_type_));
    return;
  }

  // Clamp the value as a percentage between 0 and 1
  // With this calculation, 0.5 input results 128/255 duty cycle and
  // the oscilloscope reports 50.05% duty cycle.
  // 0 input is 0 V output, 1 input is constant 3.3 V output
  // as measured by:
  // - Moritz on the 19th Sept. 2023
  // - On an ESP32-WROOM-32, pin 32
  // - RIGOL DS1102
  // - 220 ohm and difuse red LED
  value_unit.value = std::fmax(0, std::fmin(1, value_unit.value));
  const uint32_t max_value = (1 << resolution_) - 1;
  const uint32_t duty = roundf(value_unit.value * max_value);
  ledcWrite(channel_, duty);
}

bool Pwm::setup(const uint8_t pin, const uint32_t frequency,
                const uint8_t resolution) {
  // If the PWM has already been setup, free it
  if (channel_ != -1 || pin_ != -1) {
    freeResources();
  }

  // Checks if there are any free channels remaining
  if (busy_channels_.all()) {
    return true;
  }

  // Find the first free channel
  for (int i = 0; i < busy_channels_.size(); i++) {
    if (!busy_channels_.test(i)) {
      channel_ = i;
      break;
    }
  }

  // Reserve the channel as well as saving the pin and resolution
  busy_channels_[channel_] = true;
  pin_ = pin;
  resolution_ = resolution;

  // Setup the channel
  ledcSetup(channel_, frequency, resolution);

  // Attach the pin to the configured channel
  ledcAttachPin(pin_, channel_);

  return false;
}

void Pwm::freeResources() {
  if (channel_ >= 0 && channel_ < busy_channels_.size()) {
    busy_channels_[channel_] = false;
  }
  ledcDetachPin(pin_);
  pin_ = -1;
  channel_ = -1;
  resolution_ = -1;
}

const __FlashStringHelper* Pwm::pin_key_ = FPSTR("pin");
const __FlashStringHelper* Pwm::pin_key_error_ =
    FPSTR("Missing property: pin (unsigned int)");
const __FlashStringHelper* Pwm::no_channels_available_error_ =
    FPSTR("No remaining PWM channels available");

std::shared_ptr<Peripheral> Pwm::factory(const ServiceGetters& services,
                                         const JsonObjectConst& parameters) {
  return std::make_shared<Pwm>(services, parameters);
}

bool Pwm::registered_ = PeripheralFactory::registerFactory(type(), factory);

bool Pwm::capability_set_value_ = capabilities::SetValue::registerType(type());

std::bitset<16> Pwm::busy_channels_;

}  // namespace pwm
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata

#endif
