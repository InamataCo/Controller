#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include <map>
#include <memory>

#include "config.h"
#include "managers/io_types.h"
#include "managers/mqtt.h"
#include "peripheral/invalid_peripheral.h"
#include "peripheral/peripheral.h"
#include "peripheral/peripheral_factory.h"

namespace bernd_box {
namespace peripheral {

class PeripheralController {
 public:
  PeripheralController(Mqtt& mqtt, PeripheralFactory& peripheral_factory);
  void handleCallback(char* topic, uint8_t* payload, unsigned int length);

  /**
   * Returns a shared pointer to the object or a nullptr if not found
   *
   * @param name Name of the peripheral to be found
   * @return A shared pointer of the object, or a nullptr if it does not exist
   */
  std::shared_ptr<Peripheral> getPeripheral(const String& name);

 private:
  Result add(const JsonObjectConst& doc);
  Result remove(const JsonObjectConst& doc);

  Mqtt& mqtt_;
  std::map<String, std::shared_ptr<Peripheral>> peripherals_;
  PeripheralFactory& peripheral_factory_;

  const __FlashStringHelper* task_add_suffix = F("add");
  const __FlashStringHelper* task_remove_suffix = F("remove");
};

}  // namespace peripheral
}  // namespace bernd_box