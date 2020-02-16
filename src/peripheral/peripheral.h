#pragma once

#include <ArduinoJson.h>
#include <list>

namespace bernd_box {
namespace peripheral {

class TaskFactory;

class Peripheral {
 public:
  virtual ~Peripheral() = default;

  virtual const String& getType() = 0;
  const bool isValid();

 protected:
  void setInvalid();

 private:
  bool valid_ = true;
};

}  // namespace peripheral
}  // namespace bernd_box
