#ifndef PERIPHERAL_ABSTRACT_PERIPHERAL_H
#define PERIPHERAL_ABSTRACT_PERIPHERAL_H

#include <memory>

#include "ArduinoJson.h"
#include "TaskSchedulerDeclarations.h"

namespace bernd_box {

class AbstractPeripheral {
 public:
  enum class Result { kSuccess, kFailure };

  virtual ~AbstractPeripheral() {}

  virtual Result parseAction(const JsonObjectConst& doc) = 0;
  virtual bool isValid() = 0;
  virtual const __FlashStringHelper* getError() = 0;
};

}  // namespace bernd_box

#endif
