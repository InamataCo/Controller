#include "peripheral.h"

namespace bernd_box {
namespace peripheral {

bool Peripheral::isValid() const { return valid_; }

ErrorResult Peripheral::getError() const {
  if(valid_) {
    return ErrorResult();
  } else {
    return ErrorResult(getType(), error_message_);
  }
}

void Peripheral::setInvalid() { valid_ = false; }

void Peripheral::setInvalid(const String& error_message) {
  valid_ = false;
  error_message_ = error_message;
}

const __FlashStringHelper* Peripheral::data_point_type_key_ =
    F("data_point_type");
const __FlashStringHelper* Peripheral::data_point_type_key_error_ =
    F("Missing property: data_point_type (UUID)");

}  // namespace peripheral
}  // namespace bernd_box