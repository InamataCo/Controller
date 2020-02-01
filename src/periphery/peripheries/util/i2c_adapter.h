#ifndef BERND_BOX_PERIPHERY_PERIPHERIES_UTIL_I2CADAPTER_H
#define BERND_BOX_PERIPHERY_PERIPHERIES_UTIL_I2CADAPTER_H

#define PARAM_I2CADAPTER_CLOCK "scl"
#define PARAM_I2CADAPTER_DATA "sda"

#define TASK_LIST_I2C_DEVICES "listDevices"

#include <Wire.h>

#include "managers/services.h"
#include "periphery/abstract_periphery.h"

namespace bernd_box {
namespace periphery {
namespace peripheries {
namespace util {

class I2CAdapter : public AbstractPeriphery {
 public:
  I2CAdapter(const JsonObjectConst& parameter);
  virtual ~I2CAdapter();

  const String& getType() final;
  static const String& type();

  TwoWire* getWire();

 private:
  static std::shared_ptr<Periphery> factory(const JsonObjectConst&);

  static bool registered_;

  static bool wire_taken;
  static bool wire1_taken;

  bool* taken_variable;
  TwoWire* wire_;
};

class ListI2CDevicesTask : public PeripheryTask {
 public:
  Result execute();
  static const String TYPE;
};

class ListI2CDevicesTaskFactory : public TaskFactory {
 public:
  std::unique_ptr<PeripheryTask> createTask(const JsonObjectConst& parameter);
};

}  // namespace util
}  // namespace peripheries
}  // namespace periphery
}  // namespace bernd_box

#endif
