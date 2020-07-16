#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include <map>
#include <memory>

#include "managers/io_types.h"
#include "managers/server.h"
#include "peripheral/invalid_peripheral.h"
#include "peripheral/peripheral.h"
#include "peripheral/peripheral_factory.h"

namespace bernd_box {
namespace peripheral {

class PeripheralController {
 public:
  PeripheralController(Server& server, PeripheralFactory& peripheral_factory);
  void handleCallback(const JsonObjectConst& message);

  const String& type();

  /**
   * Returns a shared pointer to the object or a nullptr if not found
   *
   * @param name Name of the peripheral to be found
   * @return A shared pointer of the object, or a nullptr if it does not exist
   */
  std::shared_ptr<Peripheral> getPeripheral(const String& name);

 private:
  ErrorResult add(const JsonObjectConst& doc);
  ErrorResult remove(const JsonObjectConst& doc);

  Server& server_;
  std::map<String, std::shared_ptr<Peripheral>> peripherals_;
  PeripheralFactory& peripheral_factory_;

  const __FlashStringHelper* peripheral_command_name_ = F("peripheral");
  const __FlashStringHelper* trace_id_name_ = F("id");
  const __FlashStringHelper* add_command_name_ = F("add");
  const __FlashStringHelper* remove_command_name_ = F("remove");
};

}  // namespace peripheral
}  // namespace bernd_box