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
#include "utils/uuid.h"

namespace bernd_box {
namespace peripheral {

class PeripheralController {
 public:
  PeripheralController(Server& server, PeripheralFactory& peripheral_factory);
  void handleCallback(const JsonObjectConst& message);

  static const String& type();

  /**
   * Returns a shared pointer to the object or a nullptr if not found
   *
   * @param name Name of the peripheral to be found
   * @return A shared pointer of the object, or a nullptr if it does not exist
   */
  std::shared_ptr<Peripheral> getPeripheral(const utils::UUID& name);

 private:
  ErrorResult add(const JsonObjectConst& doc);
  ErrorResult remove(const JsonObjectConst& doc);

  Server& server_;
  std::map<utils::UUID, std::shared_ptr<Peripheral>> peripherals_;
  PeripheralFactory& peripheral_factory_;

  const __FlashStringHelper* peripheral_command_key_ = F("peripheral");
  const __FlashStringHelper* uuid_key_ = F("uuid");
  const __FlashStringHelper* uuid_key_error_ =
      F("Missing property: uuid (uuid)");
  const __FlashStringHelper* request_id_key_ = F("request_id");
  const __FlashStringHelper* add_command_key_ = F("add");
  const __FlashStringHelper* remove_command_key_ = F("remove");
};

}  // namespace peripheral
}  // namespace bernd_box