#include "peripheral.h"

namespace bernd_box {
namespace peripheral {

const bool Peripheral::isValid() { return valid_; }

void Peripheral::setInvalid() { valid_ = false; }

}  // namespace peripheral
}  // namespace bernd_box