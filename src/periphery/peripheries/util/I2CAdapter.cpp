#include "I2CAdapter.h"

#include "managers/services.h"

namespace bernd_box {
namespace periphery {
namespace peripheries {
namespace util {

const String ListI2CDevicesTask::TYPE = TASK_LIST_I2C_DEVICES;

I2CAdapter::I2CAdapter(const JsonObjectConst& parameter) {
  const char* who = __PRETTY_FUNCTION__;

  JsonVariantConst clock_pin = parameter[F(PARAM_I2CADAPTER_CLOCK)];
  if (!clock_pin.is<int>()) {
    Services::getMQTT().sendError(
        who, F("Missing property: " PARAM_I2CADAPTER_CLOCK " (int)"));
    setInvalid();
    return;
  }

  JsonVariantConst data_pin = parameter[F(PARAM_I2CADAPTER_DATA)];
  if (!data_pin.is<int>()) {
    Services::getMQTT().sendError(
        who, F("Missing property: " PARAM_I2CADAPTER_DATA " (int)"));
    setInvalid();
    return;
  }

  if (!wire_taken) {
    taken_variable = &wire_taken;
    wire_ = &Wire;
  } else if (!wire1_taken) {
    taken_variable = &wire1_taken;
    wire_ = &Wire1;
  } else {
    Services::getMQTT().sendError(who, F("Both wires already taken :("));
    setInvalid();
    return;
  }

  *taken_variable = true;
  wire_->begin(data_pin, clock_pin);
}

I2CAdapter::~I2CAdapter() { *taken_variable = false; }

TwoWire* I2CAdapter::getWire() { return wire_; }

const String& I2CAdapter::getType() { return TYPE_I2CADAPTER; }

Result ListI2CDevicesTask::execute() {}

std::unique_ptr<PeripheryTask> ListI2CDevicesTaskFactory::createTask(
    const JsonObjectConst& parameter) {}

}  // namespace util
}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box