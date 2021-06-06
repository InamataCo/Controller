#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>

#include <map>

#include "configuration.h"
#include "server.h"
#include "utils/uuid.h"

namespace bernd_box {

using namespace std::placeholders;

/**
 * Connection to the server via WebSockets
 *
 * This class creates a bi-direactional connection with the server to create
 * peripheral and tasks on the controller, and return their output to the
 * server.
 */
class WebSocket : public Server, private WebSocketsClient {
 public:
  struct Config {
    std::function<std::vector<utils::UUID>()> get_peripheral_ids;
    Server::Callback peripheral_controller_callback;
    std::function<std::vector<utils::UUID>()> get_task_ids;
    Server::Callback task_controller_callback;
    Server::Callback ota_update_callback;
    const char* core_domain;
    const char* ws_token;
    bool secure_url;
  };

  /**
   * Connection to the SDG server over websockets.
   *
   * This enables bi-directional communication between the controller and the
   * server while removing the intermediate such as the Coordinator over MQTT.
   *
   * \param config Copies out the config values to perform its initialization
   */
  WebSocket(const Config& config, String&& root_cas);
  virtual ~WebSocket() = default;

  const String& type();

  bool isConnected() final;
  bool connect(std::chrono::seconds timeout) final;

  void handle() final;

  void send(const String& name, double value) final;
  void send(const String& name, int value) final;
  void send(const String& name, bool value) final;
  void send(const String& name, DynamicJsonDocument& doc) final;
  void send(const String& name, const char* value, size_t length) final;

  void sendTelemetry(const utils::UUID& uuid, JsonObject data) final;
  void sendRegister() final;
  void sendError(const String& who, const String& message) final;
  void sendError(const ErrorResult& error, const String& request_id = "") final;

  void sendResults(JsonObjectConst results) final;
  void sendSystem(JsonObject data) final;

  const String& getRootCas() const final;

  static const __FlashStringHelper* firmware_version_;

 private:
  void handleEvent(WStype_t type, uint8_t* payload, size_t length);
  void handleData(const uint8_t* payload, size_t length);

  void restartOnUnimplementedFunction();

  bool is_setup_ = false;

  std::function<std::vector<utils::UUID>()> get_peripheral_ids_;
  Callback peripheral_controller_callback_;
  std::function<std::vector<utils::UUID>()> get_task_ids_;
  Callback task_controller_callback_;
  Callback ota_update_callback_;

  String core_domain_;
  bool secure_url_;
  const char* controller_path_ = "/ws-api/v1/farms/controllers/";
  String ws_token_;
  String root_cas_;
};

}  // namespace bernd_box