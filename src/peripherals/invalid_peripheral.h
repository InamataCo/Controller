#ifndef PERIPHERALS_INVALID_PERIPHERAL_H
#define PERIPHERALS_INVALID_PERIPHERAL_H

#include "abstract_peripheral.h"

namespace bernd_box {

class InvalidPeripheral : public AbstractPeripheral {
 public:
  InvalidPeripheral(const __FlashStringHelper* error_msg = F("Null peripheral"));
  virtual ~InvalidPeripheral() = default;

  Result parseAction(const JsonObjectConst& doc) final;
  bool isValid() final;
  const __FlashStringHelper* getError() final;

 private:
  const __FlashStringHelper* error_msg_;
};

}  // namespace bernd_box

#endif
