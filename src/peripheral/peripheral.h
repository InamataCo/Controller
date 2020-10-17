#pragma once

#include <Arduino.h>

#include <array>

#include "managers/io_types.h"

namespace bernd_box {
namespace peripheral {

class TaskFactory;

class Peripheral {
 public:
  virtual ~Peripheral() = default;

  virtual const String& getType() const = 0;
  
  bool isValid() const;
  ErrorResult getError() const;

 protected:
  void setInvalid();
  void setInvalid(const String& error_message);

 private:
  bool valid_ = true;
  String error_message_;
};

}  // namespace peripheral
}  // namespace bernd_box
