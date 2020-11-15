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

  static const String& type();
  
  void handleCallback(const JsonObjectConst& message);

  /**
   * Returns a list of all peripherals' IDs
   * 
   * \return A list of all peripherals' IDs
   */
  std::vector<utils::UUID> getPeripheralIDs();

  /**
   * Returns a shared pointer to the object or a nullptr if not found
   *
   * @param name Name of the peripheral to be found
   * @return A shared pointer of the object, or a nullptr if it does not exist
   */
  std::shared_ptr<Peripheral> getPeripheral(const utils::UUID& name);

 private:
  /**
   * Create a new peripheral according to the JSON doc
   *
   * \param doc The JSON doc with the parameters to create a peripheral
   * \return Contains the source and cause of the error, if it failed
   */
  ErrorResult add(const JsonObjectConst& doc);

  /**
   * Remove a peripheral by its UUID
   *
   * \param doc The JSON doc containing the UUID of the peripheral to remove
   * \return Contains the source and cause of the error, if one occured
   */
  ErrorResult remove(const JsonObjectConst& doc);

  static void addResultEntry(const JsonVariantConst& uuid,
                             const ErrorResult& error,
                             const JsonArray& results);

  /// The server to which to reply to
  Server& server_;
  /// Map of UUIDs to their respective peripherals
  std::map<utils::UUID, std::shared_ptr<Peripheral>> peripherals_;
  /// Factory to construct peripherals according to the JSON parameters
  PeripheralFactory& peripheral_factory_;

  static const __FlashStringHelper* peripheral_command_key_;
  static const __FlashStringHelper* uuid_key_;
  static const __FlashStringHelper* uuid_key_error_;
  static const __FlashStringHelper* add_command_key_;
  static const __FlashStringHelper* remove_command_key_;
};

}  // namespace peripheral
}  // namespace bernd_box