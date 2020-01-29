#ifndef PERIPHERALS_L293D_MOTORS_H
#define PERIPHERALS_L293D_MOTORS_H

#include "abstract_peripheral.h"

namespace bernd_box {

class L293dMotor : public AbstractPeripheral {
 public:
  L293dMotor();
  Result parseAction(const JsonObjectConst& doc) final;
};

}  // namespace bernd_box

#endif
