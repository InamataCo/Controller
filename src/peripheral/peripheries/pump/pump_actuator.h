#ifndef BERND_BOX_TASKS_PUMP_H
#define BERND_BOX_TASKS_PUMP_H

#include <ArduinoJson.h>

#include "managers/services.h"
#include "peripheral/capabilities/set_value.h"
#include "peripheral/peripheral.h"

namespace bernd_box {
namespace peripheral {
namespace peripheries {
namespace pump {

class PumpActuator : public Peripheral, public capabilities::SetValue {
 public:
  PumpActuator(const JsonObjectConst& parameters);
  virtual ~PumpActuator() = default;

  // Type registration in the periphery factory / library
  const String& getType() final;
  static const String& type();

  // Capability: SetValue
  void setValue(float value) final;

  /// Name of the parameter for the pump to control a pump
  static const __FlashStringHelper* pump_pin_name_;

 private:
  static std::shared_ptr<Peripheral> factory(const JsonObjectConst& parameter);
  static bool registered_;
  static bool capability_set_value_;

  unsigned int pump_pin_;
};

}  // namespace pump
}  // namespace peripheries
}  // namespace peripheral
}  // namespace bernd_box

#endif
