#pragma once

#include <Wire.h>

#include "managers/services.h"
#include "peripheral/peripheral.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace util {

/**
 * The driver for an I2C interface that supports both hardware I2C controllers
 */
class I2CAdapter : public Peripheral {
 public:
  I2CAdapter(const JsonObjectConst& parameter);
  virtual ~I2CAdapter();

  const String& getType() final;
  static const String& type();

  TwoWire* getWire();

 private:
  static std::shared_ptr<Peripheral> factory(const JsonObjectConst&);

  static bool registered_;

  static bool wire_taken;
  static bool wire1_taken;

  bool* taken_variable;
  TwoWire* wire_;
};

}  // namespace util
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
