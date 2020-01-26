#include "invalid_peripheral.h"

namespace bernd_box {

InvalidPeripheral::InvalidPeripheral(const __FlashStringHelper* error_msg)
    : error_msg_(error_msg) {}

InvalidPeripheral::Result InvalidPeripheral::parseAction(const JsonObjectConst& doc) {
  return Result::kFailure;
}

bool InvalidPeripheral::isValid() { return false; }

const __FlashStringHelper* InvalidPeripheral::getError() { return error_msg_; }

}  // namespace bernd_box
