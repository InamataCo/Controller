#include "peripheral.h"

namespace inamata {
namespace peripheral {

bool Peripheral::isValid() const { return valid_; }

ErrorResult Peripheral::getError() const {
  if (valid_) {
    return ErrorResult();
  } else {
    return ErrorResult(getType(), error_message_);
  }
}

int Peripheral::toPin(JsonVariantConst pin) {
  if (!pin.is<float>()) {
    return -1;
  }
  int pin_number = pin.as<int>();
  if (pin_number < 0 || pin_number > 255) {
    return -1;
  }
  return pin_number;
}

void Peripheral::setInvalid() { valid_ = false; }

void Peripheral::setInvalid(const String& error_message) {
  valid_ = false;
  error_message_ = error_message;
}

const __FlashStringHelper* Peripheral::data_point_type_key_ =
    FPSTR("data_point_type");
const __FlashStringHelper* Peripheral::data_point_type_key_error_ =
    FPSTR("Missing property: data_point_type (UUID)");

}  // namespace peripheral
}  // namespace inamata