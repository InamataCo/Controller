#pragma once

#include "periphery_factory.h"
#include "peripheral/peripheral.h"

namespace bernd_box {
namespace peripheral {

class InvalidPeriphery : public Peripheral {
 public:
  virtual ~InvalidPeriphery() = default;

  const String& getType() final;
  static const String& type();

 private:
  static std::shared_ptr<Peripheral> factory(const JsonObjectConst&);
  
  static bool registered_;
};

}  // namespace peripheral
}  // namespace bernd_box