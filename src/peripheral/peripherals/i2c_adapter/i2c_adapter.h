#pragma once

#include <Wire.h>

#include "managers/service_getters.h"
#include "peripheral/peripheral.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace util {

/**
 * The driver for an I2C interface that supports both hardware I2C controllers
 */
class I2CAdapter : public Peripheral {
 public:
  I2CAdapter(const ServiceGetters& services, const JsonObjectConst& parameter);
  virtual ~I2CAdapter();

  const String& getType() const final;
  static const String& type();

  TwoWire* getWire();

 private:
  static std::shared_ptr<Peripheral> factory(const ServiceGetters& services,
                                             const JsonObjectConst&);

  static bool registered_;

  static bool wire_taken;
  static bool wire1_taken;

  std::shared_ptr<Server> server_;

  bool* taken_variable;
  TwoWire* wire_;
};

}  // namespace util
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata
