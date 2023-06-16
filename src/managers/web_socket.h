#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>

#include <functional>
#include <map>
#include <vector>

#include "configuration.h"
#include "managers/logging.h"
#include "utils/uuid.h"

namespace inamata {

using namespace std::placeholders;

/**
 * Connection to the server via WebSockets
 *
 * This class creates a bi-direactional connection with the server to create
 * peripheral and tasks on the controller, and return their output to the
 * server.
 */
class WebSocket {
 public:
  enum class ConnectState { kConnected, kConnecting, kFailed };

  using Callback = std::function<void(const JsonObjectConst& message)>;
  using CallbackMap = std::map<String, Callback>;

  struct Config {
    std::function<std::vector<utils::UUID>()> get_peripheral_ids;
    Callback peripheral_controller_callback;
    std::function<std::vector<utils::UUID>()> get_task_ids;
    Callback task_controller_callback;
    Callback ota_update_callback;
    const char* core_domain;
    const char* ws_token;
    bool secure_url;
  };

  /**
   * Connection to the Inamata server over websockets.
   *
   * This enables bi-directional communication between the controller and the
   * server while removing the intermediate such as the Coordinator over MQTT.
   *
   * \param config Copies out the config values to perform its initialization
   */
  WebSocket(const Config& config, String&& root_cas);
  virtual ~WebSocket() = default;

  const String& type();

  ConnectState handle();

  /**
   * Resets the last connect time to start a new connect attempt
   */
  void resetConnectAttempt();

  // void send(const String& name, double value);
  // void send(const String& name, int value);
  // void send(const String& name, bool value);
  // void send(const String& name, JsonDocument& doc);
  // void send(const String& name, const char* value, size_t length);

  void sendTelemetry(const utils::UUID& uuid, JsonObject data);
  void sendRegister();
  void sendError(const String& who, const String& message);
  void sendError(const ErrorResult& error, const String& request_id = "");
  
  void sendDebug(const String& message);

  void sendResults(JsonObjectConst results);
  void sendSystem(JsonObject data);

  const char* getRootCas() const;
  void setWsToken(const char* token);
  const bool isWsTokenSet() const;

  static const __FlashStringHelper* firmware_version_;
  String core_domain_;
  static const char* core_domain_key_;
  bool secure_url_;
  static const char* secure_url_key_;
  static const char* ws_token_key_;

  static const __FlashStringHelper* request_id_key_;
  static const __FlashStringHelper* type_key_;
  static const __FlashStringHelper* result_type_;
  static const __FlashStringHelper* telemetry_type_;
  static const __FlashStringHelper* task_key_;
  static const __FlashStringHelper* system_type_;

 private:
  /**
   * Checks if the WebSocket connected to the server
   *
   * @return true if connected
   */
  bool isConnected();

  /**
   * Perform setup and check if connect timeout has been reached
   *
   * @return ConnectState kConnecting if retrying, kFailed if timed out
   */
  ConnectState connect();

  void handleEvent(WStype_t type, uint8_t* payload, size_t length);
  void handleData(const uint8_t* payload, size_t length);

  /**
   * Save the up/down durations and timepoints when the connection state changes
   *
   * @param is_connected True if the connection is currently connected
   */
  void updateUpDownTime(const bool is_connected);
  void sendUpDownTimeData();

  void restartOnUnimplementedFunction();

  bool is_setup_ = false;

  /// Whether the WebSocket was connected during the last check
  bool was_connected_ = false;
  bool send_on_connect_messages_ = false;
  /// The timepoint when the last connect attempt started
  std::chrono::steady_clock::time_point last_connect_start_ =
      std::chrono::steady_clock::time_point::min();
  /// The timepoint when the connection last went up
  std::chrono::steady_clock::time_point last_connect_up_ =
      std::chrono::steady_clock::time_point::min();
  /// The timepoint when the connection last went down
  std::chrono::steady_clock::time_point last_connect_down_ =
      std::chrono::steady_clock::time_point::min();
  /// The duration of the last WebSocket connection uptime
  std::chrono::steady_clock::duration last_up_duration_ =
      std::chrono::steady_clock::duration::min();
  /// The duration of the last WebSocket connection downtime
  std::chrono::steady_clock::duration last_down_duration_ =
      std::chrono::steady_clock::duration::min();

  std::function<std::vector<utils::UUID>()> get_peripheral_ids_;
  Callback peripheral_controller_callback_;
  std::function<std::vector<utils::UUID>()> get_task_ids_;
  Callback task_controller_callback_;
  Callback ota_update_callback_;

  String root_cas_;
  String ws_token_;
  const char* controller_path_ = "/controller-ws/v1/";
};

}  // namespace inamata