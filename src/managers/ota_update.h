#pragma once

#include <ArduinoJson.h>

#include "managers/service_getters.h"
#include "tasks/base_task.h"
#include "utils/error_store.h"

namespace bernd_box {

/**
 * Service to perform OTA updates
 */
class OtaUpdate : public tasks::BaseTask {
 public:
  OtaUpdate(Scheduler& scheduler);
  virtual ~OtaUpdate() = default;

  static const String& type();

  void setServices(ServiceGetters services);

  /**
   * Handle the command to update the firmware
   *
   * \param message Information regarding the firmware update
   */
  void handleCallback(const JsonObjectConst& message);

  /**
   * Download the firmware image as a stream and write it to the flash
   */
  bool TaskCallback();

 private:
  void sendResult(const __FlashStringHelper* status, const String& error = "");

  /// The server to which to reply to
  ServiceGetters services_;

  /// The request ID used by the update command
  String request_id_;

  /// Size of the image to be downloaded
  int image_size_ = 0;

  esp_https_ota_config_t ota_config_;
  esp_http_client_config_t ota_http_config_;
  esp_https_ota_handle_t ota_handle_;

  static const __FlashStringHelper* update_command_key_;
  static const __FlashStringHelper* url_key_;
  static const __FlashStringHelper* image_size_key_;

  static const __FlashStringHelper* status_key_;
  static const __FlashStringHelper* status_start_;
  static const __FlashStringHelper* status_updating_;
  static const __FlashStringHelper* status_finish_;
  static const __FlashStringHelper* status_fail_;
  static const __FlashStringHelper* detail_key_;
  static const __FlashStringHelper* start_failed_error_;
  static const __FlashStringHelper* updating_failed_error_;
};

}  // namespace bernd_box
