#pragma once

#include <memory>

#include "managers/service_getters.h"
#include "peripheral/peripheral.h"

namespace bernd_box {
namespace peripheral {

class InvalidPeripheral : public Peripheral {
 public:
  InvalidPeripheral();
  InvalidPeripheral(const String& error);
  virtual ~InvalidPeripheral() = default;

  const String& getType() const final;
  static const String& type();

 private:
  static std::shared_ptr<Peripheral> factory(const ServiceGetters& services,
                                             const JsonObjectConst&);

  static bool registered_;
};

}  // namespace peripheral
}  // namespace bernd_box