#include "i2c_adapter.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace util {

bool I2CAdapter::wire_taken = false;
bool I2CAdapter::wire1_taken = false;

I2CAdapter::I2CAdapter(const JsonObjectConst& parameter) {
  const char* who = __PRETTY_FUNCTION__;

  JsonVariantConst clock_pin = parameter[F(PARAM_I2CADAPTER_CLOCK)];
  if (!clock_pin.is<int>()) {
    Services::getMqtt().sendError(
        who, F("Missing property: " PARAM_I2CADAPTER_CLOCK " (int)"));
    setInvalid();
    return;
  }

  JsonVariantConst data_pin = parameter[F(PARAM_I2CADAPTER_DATA)];
  if (!data_pin.is<int>()) {
    Services::getMqtt().sendError(
        who, F("Missing property: " PARAM_I2CADAPTER_DATA " (int)"));
    setInvalid();
    return;
  }

  if (!wire_taken) {
    taken_variable = &wire_taken;
    wire_ = &Wire;
  } else if (!wire1_taken) {
    taken_variable = &wire1_taken;
    wire_ = &Wire1;
  } else {
    Services::getMqtt().sendError(who, F("Both wires already taken :("));
    setInvalid();
    return;
  }

  *taken_variable = true;
  wire_->begin(data_pin, clock_pin);
}

I2CAdapter::~I2CAdapter() { *taken_variable = false; }

const String& I2CAdapter::getType() { return type(); }

const String& I2CAdapter::type() {
  static const String name{"I2CAdapter"};
  return name;
}

TwoWire* I2CAdapter::getWire() { return wire_; }

std::shared_ptr<Peripheral> I2CAdapter::factory(
    const JsonObjectConst& parameter) {
  return std::make_shared<I2CAdapter>(parameter);
}

}  // namespace util
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
