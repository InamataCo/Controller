#include "ota_update.h"

#include <chrono>
#include <memory>

#include "ArduinoJson.h"
#include "Update.h"
#include "esp_https_ota.h"

namespace bernd_box {

const String& OtaUpdate::type() {
  static const String name{"OtaUpdate"};
  return name;
}

void OtaUpdate::handleCallback(const JsonObjectConst& message) {
  JsonVariantConst update_command = message[update_command_key_];
  if (!update_command) {
    return;
  }
  request_id_ = message[Server::request_id_key_].as<const char*>();

  JsonVariantConst url = update_command[url_key_];
  if (!url.is<const char*>()) {
    String error =
        ErrorStore::genMissingProperty(url_key_, ErrorStore::KeyType::kString);
    sendResult(status_fail_, error);
    return;
  }

  JsonVariantConst image_size = update_command[image_size_key_];
  if (!image_size.is<size_t>()) {
    String error =
        ErrorStore::genMissingProperty(url_key_, ErrorStore::KeyType::kString);
    sendResult(status_fail_, error);
    return;
  }
  image_size_ = image_size;

  ota_http_config_.url = url.as<const char*>();
  ota_http_config_.use_global_ca_store = true;
  ota_config_.http_config = &ota_http_config_;
  esp_err_t err = esp_https_ota_begin(&ota_config_, &ota_handle_);

  if (err == ESP_OK) {
    sendResult(status_start_);
  } else {
    sendResult(status_fail_, start_failed_error_);
    return;
  }

  setIterations(-1);
  setInterval(std::chrono::milliseconds(100).count());
  enable();
}

bool OtaUpdate::TaskCallback() {
  esp_err_t err = esp_https_ota_perform(ota_handle_);
  if (err == ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
    String percent =
        String(esp_https_ota_get_image_len_read(ota_handle_) / image_size_);
    percent += "%";
    sendResult(status_updating_, percent);
    return true;
  } else if (err == ESP_OK &&
             esp_https_ota_is_complete_data_received(ota_handle_) &&
             esp_https_ota_finish(ota_handle_)) {
    sendResult(status_finish_);
    esp_restart();
    return false;
  } else {
    sendResult(status_fail_, updating_failed_error_);
    return false;
  }
}

void OtaUpdate::sendResult(const __FlashStringHelper* status,
                           const String& detail) {
  // Get the server instance to send the error message
  std::shared_ptr<Server> server = services_.get_server();
  if (server == nullptr) {
    Serial.println(
        ErrorResult(type(), ServiceGetters::nullptr_error_).toString());
    return;
  }

  // Create the error message
  DynamicJsonDocument result_doc(BB_JSON_PAYLOAD_SIZE);
  result_doc[Server::type_key_] = Server::result_type_;
  if (!request_id_.isEmpty()) {
    result_doc[Server::request_id_key_] = request_id_.c_str();
  }
  JsonObject update_result = result_doc.createNestedObject(update_command_key_);
  update_result[status_key_] = status;
  if (!detail.isEmpty()) {
    update_result[detail_key_] = detail.c_str();
  }

  // Send the error and clear the request ID to end trace line
  server->sendResults(result_doc.as<JsonObject>());
  request_id_.clear();
}

const __FlashStringHelper* OtaUpdate::update_command_key_ = F("update");
const __FlashStringHelper* OtaUpdate::url_key_ = F("url");
const __FlashStringHelper* OtaUpdate::image_size_key_ = F("size");

const __FlashStringHelper* OtaUpdate::status_key_ = F("status");
const __FlashStringHelper* OtaUpdate::status_start_ = F("start");
const __FlashStringHelper* OtaUpdate::status_updating_ = F("updating");
const __FlashStringHelper* OtaUpdate::status_finish_ = F("finish");
const __FlashStringHelper* OtaUpdate::status_fail_ = F("fail");
const __FlashStringHelper* OtaUpdate::detail_key_ = F("detail");
const __FlashStringHelper* OtaUpdate::start_failed_error_ =
    F("OTA start failed");
const __FlashStringHelper* OtaUpdate::updating_failed_error_ =
    F("OTA update failed");

}  // namespace bernd_box