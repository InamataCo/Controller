#pragma once
#ifdef ESP32

#include <ArduinoJson.h>
#ifdef ESP32
#include <HTTPClient.h>
#include <esp_https_ota.h>
#elif ESP8266
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#else
#error Only ESP32 and ESP8266 supported
#endif


#include "managers/service_getters.h"
#include "tasks/base_task.h"
#include "utils/error_store.h"

namespace inamata {

/**
 * Service to perform OTA updates
 */
class OtaUpdater : public tasks::BaseTask {
 public:
  OtaUpdater(Scheduler& scheduler);
  virtual ~OtaUpdater() = default;

  const String& getType() const final;
  static const String& type();

  void setServices(ServiceGetters services);

  /**
   * Handle the command to update the firmware
   *
   * \param message Information regarding the firmware update
   */
  void handleCallback(const JsonObjectConst& message);

  /**
   * Acquire a lock to prevent parallel updates
   */
  bool OnTaskEnable();

  /**
   * Download the firmware image as a stream and write it to the flash
   */
  bool TaskCallback();

  /**
   * Perform clean up for next OTA update attempt
   *
   * Free the buffer and release a lock to allow the next update request.
   */
  void OnTaskDisable();

 private:
  /**
   *
   * \return Returns true on success
   */
  ErrorResult connectToServer(const String& url);

  void sendResult(const __FlashStringHelper* status, const String& error = "",
                  const char* request_id = nullptr);

  /// The server to which to reply to
  ServiceGetters services_;

  /// The request ID used by the update command
  String request_id_;

  /// A lock when an update is in progress
  bool is_updating_ = false;

  /// The last update progress percentage
  int last_percent_update = -1;

  /// Whether to restart on success. Ignored on update fail
  bool restart_;

  /// Size of the image to be downloaded
  int32_t image_size_ = 0;
  std::vector<uint8_t> buffer_;

  HTTPClient client_;

  esp_http_client_config_t ota_http_config_;
  esp_https_ota_config_t ota_config_;
  esp_https_ota_handle_t ota_handle_;

  static const __FlashStringHelper* update_command_key_;
  static const __FlashStringHelper* url_key_;
  static const __FlashStringHelper* image_size_key_;
  static const __FlashStringHelper* md5_hash_key_;
  static const __FlashStringHelper* restart_key_;

  static const __FlashStringHelper* status_key_;
  static const __FlashStringHelper* status_start_;
  static const __FlashStringHelper* status_updating_;
  static const __FlashStringHelper* status_finish_;
  static const __FlashStringHelper* status_fail_;
  static const __FlashStringHelper* detail_key_;
  static const __FlashStringHelper* failed_to_connect_error_;
  static const __FlashStringHelper* connection_lost_error_;
  static const __FlashStringHelper* update_in_progress_error_;
  static const __FlashStringHelper* http_code_error_;
};

}  // namespace inamata

#endif