#include "peripheral.h"

namespace bernd_box {
namespace peripheral {

bool Peripheral::isValid() const { return valid_; }

ErrorResult Peripheral::getError() const {
  return ErrorResult(getType(), error_message_);
}

void Peripheral::setInvalid() { valid_ = false; }

void Peripheral::setInvalid(const String& error_message) {
  valid_ = false;
  error_message_ = error_message;
}

}  // namespace peripheral
}  // namespace bernd_box