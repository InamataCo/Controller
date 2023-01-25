#ifdef ESP32
#include "ota_updater.h"

#include <ArduinoJson.h>
#include <Update.h>

#include <chrono>
#include <memory>

namespace inamata {

OtaUpdater::OtaUpdater(Scheduler& scheduler) : BaseTask(scheduler) {}

const String& OtaUpdater::getType() const { return type(); }

const String& OtaUpdater::type() {
  static const String name{"OtaUpdater"};
  return name;
}

void OtaUpdater::setServices(ServiceGetters services) { services_ = services; }

void OtaUpdater::handleCallback(const JsonObjectConst& message) {
  JsonVariantConst update_command = message[update_command_key_];
  if (!update_command) {
    return;
  }

  // Abort if an update is already running and use the new request ID
  const char* request_id =
      message[WebSocket::request_id_key_].as<const char*>();
  if (is_updating_) {
    sendResult(status_fail_, update_in_progress_error_, request_id);
    return;
  }
  request_id_ = request_id;

  JsonVariantConst url = update_command[url_key_];
  if (!url.is<const char*>()) {
    sendResult(status_fail_, ErrorStore::genMissingProperty(
                                 url_key_, ErrorStore::KeyType::kString));
    return;
  }
  const char* url_str = url.as<const char*>();

  JsonVariantConst image_size = update_command[image_size_key_];
  if (!image_size.is<size_t>()) {
    sendResult(status_fail_,
               ErrorStore::genMissingProperty(image_size_key_,
                                              ErrorStore::KeyType::kString));
    return;
  }
  image_size_ = image_size;

  JsonVariantConst md5_hash = update_command[md5_hash_key_];
  if (!md5_hash.is<const char*>()) {
    sendResult(status_fail_, ErrorStore::genMissingProperty(
                                 md5_hash_key_, ErrorStore::KeyType::kString));
    return;
  }

  JsonVariantConst restart = update_command[restart_key_];
  if (!restart.is<bool>()) {
    sendResult(status_fail_, ErrorStore::genMissingProperty(
                                 restart_key_, ErrorStore::KeyType::kBool));
    return;
  }
  restart_ = restart;

  // Check if to use TLS or not
  bool connected;
  if (strcmp(url_str, "https") == 0) {
    // Get and set the CA if available
    std::shared_ptr<WebSocket> web_socket = services_.getWebSocket();
    if (web_socket == nullptr) {
      TRACELN(
          ErrorResult(type(), ServiceGetters::web_socket_nullptr_error_)
              .toString());
      return;
    }
    connected = client_.begin(url.as<const char*>(), web_socket->getRootCas());
  } else {
    // Use an unencrypted connection
    connected = client_.begin(url.as<const char*>());
  }
  if (!connected) {
    sendResult(status_fail_, failed_to_connect_error_);
    return;
  }

  // Start the HTTP GET request
  int status_code = client_.GET();
  if (status_code != HTTP_CODE_OK) {
    sendResult(
        status_fail_,
        ErrorResult(type(), String(http_code_error_) + status_code).toString());
    return;
  }

  // Initialize the firmware update driver
  Update.setMD5(md5_hash.as<const char*>());
  Update.begin(image_size_);

  // Confirm to the server that the update process has started
  sendResult(status_start_, "");

  setIterations(-1);
  setInterval(std::chrono::milliseconds(50).count());
  enable();
}

bool OtaUpdater::OnTaskEnable() {
  buffer_.resize(4096);
  is_updating_ = true;
  return true;
}

bool OtaUpdater::TaskCallback() {
  WiFiClient* stream = client_.getStreamPtr();
  if (stream == nullptr) {
    sendResult(status_fail_, connection_lost_error_);
    return false;
  }

  if (stream->available()) {
    int bytes_read = stream->readBytes(buffer_.data(), buffer_.size());
    // bytes_read = stream.read(buffer_.data(), buffer_.size());
    Update.write(buffer_.data(), bytes_read);

    // Inform the server of progress
    if (bytes_read) {
      int percent = float(Update.progress()) / float(image_size_) * 100;
      if (last_percent_update < 0 || last_percent_update + 10 <= percent) {
        last_percent_update = percent;
        String status = F("{done:");
        status += last_percent_update;
        status += F("%}");
        sendResult(status_updating_, status);
      }
    }

    // Check if the update is finished, successfully or errored out
    if (Update.isFinished()) {
      // Close the HTTP connection
      client_.end();

      // Mark the update as complete and check if it succeeded
      bool success = Update.end();
      if (success) {
        // If successful, inform the server and reboot
        sendResult(status_finish_, "");
      } else {
        // If not, inform the server and remove the updating lock
        sendResult(status_fail_, Update.errorString());
        restart_ = false;
      }
      disable();
    }
  }
  return true;
}

void OtaUpdater::OnTaskDisable() {
  if (restart_) {
    // Sleep to allow remaining packets to be sent
    ::delay(1500);
    esp_restart();
  }
  buffer_.clear();
  buffer_.shrink_to_fit();
  request_id_.clear();
  last_percent_update = -1;
  is_updating_ = false;
}

void OtaUpdater::sendResult(const __FlashStringHelper* status,
                            const String& detail, const char* request_id) {
  // Get the server instance to send the error message
  std::shared_ptr<WebSocket> web_socket = services_.getWebSocket();
  if (web_socket == nullptr) {
    TRACELN(
        ErrorResult(type(), ServiceGetters::web_socket_nullptr_error_)
            .toString());
    return;
  }
  // Create the error message
  doc_out.clear();
  doc_out[WebSocket::type_key_] = WebSocket::result_type_;
  // Favor request ID from parameters over class member. Ignore if none given
  if (request_id != nullptr) {
    doc_out[WebSocket::request_id_key_] = request_id;
  } else if (!request_id_.isEmpty()) {
    doc_out[WebSocket::request_id_key_] = request_id_.c_str();
  }

  JsonObject update_result = doc_out.createNestedObject(update_command_key_);
  update_result[status_key_] = status;
  if (!detail.isEmpty()) {
    update_result[detail_key_] = detail.c_str();
  }
  // Send the error and clear the request ID to end trace line
  web_socket->sendResults(doc_out.as<JsonObject>());
}

const __FlashStringHelper* OtaUpdater::update_command_key_ = FPSTR("update");
const __FlashStringHelper* OtaUpdater::url_key_ = FPSTR("url");
const __FlashStringHelper* OtaUpdater::image_size_key_ = FPSTR("size");
const __FlashStringHelper* OtaUpdater::md5_hash_key_ = FPSTR("md5");
const __FlashStringHelper* OtaUpdater::restart_key_ = FPSTR("restart");

const __FlashStringHelper* OtaUpdater::status_key_ = FPSTR("status");
const __FlashStringHelper* OtaUpdater::status_start_ = FPSTR("start");
const __FlashStringHelper* OtaUpdater::status_updating_ = FPSTR("updating");
const __FlashStringHelper* OtaUpdater::status_finish_ = FPSTR("finish");
const __FlashStringHelper* OtaUpdater::status_fail_ = FPSTR("fail");
const __FlashStringHelper* OtaUpdater::detail_key_ = FPSTR("detail");
const __FlashStringHelper* OtaUpdater::failed_to_connect_error_ =
    FPSTR("Failed to connect");
const __FlashStringHelper* OtaUpdater::connection_lost_error_ =
    FPSTR("Connection lost");
const __FlashStringHelper* OtaUpdater::update_in_progress_error_ =
    FPSTR("OTA update already running");
const __FlashStringHelper* OtaUpdater::http_code_error_ = FPSTR("HTTP code: ");

}  // namespace inamata

#endif