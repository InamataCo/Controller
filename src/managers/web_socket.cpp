#include "web_socket.h"

#ifdef ESP32
#include <esp_tls.h>
#endif

namespace inamata {

WebSocketsClient websocket_client;

WebSocket::WebSocket(const WebSocket::Config& config, String&& root_cas)
    : core_domain_(config.core_domain),
      secure_url_(config.secure_url),
      get_peripheral_ids_(config.get_peripheral_ids),
      peripheral_controller_callback_(config.peripheral_controller_callback),
      get_task_ids_(config.get_task_ids),
      task_controller_callback_(config.task_controller_callback),
      ota_update_callback_(config.ota_update_callback),
      root_cas_(root_cas) {
  if (core_domain_.isEmpty()) {
    core_domain_ = F("core.staging.inamata.co");
    secure_url_ = true;
  }
  if (config.ws_token) {
    setWsToken(config.ws_token);
  }
}

const String& WebSocket::type() {
  static const String name{"WebSocket"};
  return name;
}

bool WebSocket::isConnected() {
  const bool is_connected = websocket_client.isConnected();
  updateUpDownTime(is_connected);
  return is_connected;
}

WebSocket::ConnectState WebSocket::connect() {
  // Configure the WebSocket interface with the server, TLS certificate and the
  // reconnect interval
  if (!is_setup_) {
    TRACELN(F("Setting up"));
    is_setup_ = true;
    if (last_connect_up_ == last_connect_up_.min()) {
      last_connect_up_ = std::chrono::steady_clock::now();
    }
    // WS token has to be set in LittleFS, EEPROM or via captive portal
    if (!isWsTokenSet()) {
      TRACELN(F("ws_token not set"));
      return ConnectState::kFailed;
    }
    if (secure_url_) {
      websocket_client.beginSslWithCA(core_domain_.c_str(), 443,
                                      controller_path_, getRootCas(),
                                      ws_token_.c_str());
    } else {
      websocket_client.begin(core_domain_.c_str(), 8000, controller_path_,
                             ws_token_.c_str());
    }
    websocket_client.onEvent(
        std::bind(&WebSocket::handleEvent, this, _1, _2, _3));
    websocket_client.setReconnectInterval(5000);
  }

  if (last_connect_start_ == last_connect_start_.min()) {
    last_connect_start_ = std::chrono::steady_clock::now();
  }

  // If timed out, say failed, else attempt to connect
  if (std::chrono::steady_clock::now() - last_connect_start_ >
      web_socket_connect_timeout) {
    return ConnectState::kFailed;
  }
  websocket_client.loop();
  return ConnectState::kConnecting;
}

WebSocket::ConnectState WebSocket::handle() {
  if (isConnected()) {
    // On reconnect, send register and other messages
    if (send_on_connect_messages_) {
      TRACELN(F("Reconnected to server"));
      send_on_connect_messages_ = false;
      sendRegister();
      sendUpDownTimeData();
    }
    websocket_client.loop();
    return ConnectState::kConnected;
  }

  return connect();
}

void WebSocket::resetConnectAttempt() {
  last_connect_start_ = last_connect_start_.min();
}

// void WebSocket::send(const String& name, double value) {
//   restartOnUnimplementedFunction();
// }
// void WebSocket::send(const String& name, int value) {
//   restartOnUnimplementedFunction();
// }
// void WebSocket::send(const String& name, bool value) {
//   restartOnUnimplementedFunction();
// }

// void WebSocket::send(const String& name, JsonDocument& doc) {
//   doc["type"] = "tel";
//   doc["name"] = name;

//   // Calculate the size of the resultant serialized JSON, create a buffer of
//   // that size and serialize the JSON into that buffer.
//   // Add extra byte for the null terminator
//   std::vector<char> register_buf = std::vector<char>(measureJson(doc) + 1);
//   size_t n = serializeJson(doc, register_buf.data(), register_buf.size());

//   websocket_client.sendTXT(register_buf.data(), n);
// }

// void WebSocket::send(const String& name, const char* value, size_t length) {
//   restartOnUnimplementedFunction();
// }

void WebSocket::sendTelemetry(const utils::UUID& task_id, JsonObject data) {
  data[WebSocket::type_key_] = WebSocket::telemetry_type_;
  data[WebSocket::task_key_] = task_id.toString();

  std::vector<char> register_buf = std::vector<char>(measureJson(data) + 1);
  size_t n = serializeJson(data, register_buf.data(), register_buf.size());

  websocket_client.sendTXT(register_buf.data(), n);
}

void WebSocket::sendRegister() {
  doc_out.clear();

  // Use the register message type
  doc_out["type"] = "reg";

  // Set the firmware version number
  doc_out["version"] = firmware_version_;

  // Collect all added peripheral ids and write them to a JSON doc
  std::vector<utils::UUID> peripheral_ids = get_peripheral_ids_();
  if (!peripheral_ids.empty()) {
    JsonArray peripherals = doc_out.createNestedArray(F("peripherals"));
    for (const auto& peripheral_id : peripheral_ids) {
      peripherals.add(peripheral_id.toString());
    }
  }

  // Collect all running task ids and write them to a JSON doc
  std::vector<utils::UUID> task_ids = get_task_ids_();
  if (!task_ids.empty()) {
    JsonArray tasks = doc_out.createNestedArray(F("tasks"));
    for (const auto& task_id : task_ids) {
      if (task_id.isValid()) {
        tasks.add(task_id.toString());
      }
    }
  }

  // Calculate the size of the resultant serialized JSON, create a buffer of
  // that size and serialize the JSON into that buffer.
  // Add extra byte for the null terminator
  std::vector<char> register_buf = std::vector<char>(measureJson(doc_out) + 1);
  size_t n = serializeJson(doc_out, register_buf.data(), register_buf.size());

  websocket_client.sendTXT(register_buf.data(), n);
}

void WebSocket::sendError(const String& who, const String& message) {
  TRACELN(message);
  doc_out.clear();

  // Use ther error message type
  doc_out["type"] = "err";

  // Place the error message
  doc_out["message"] = message.c_str();

  std::vector<char> register_buf = std::vector<char>(measureJson(doc_out) + 1);
  size_t n = serializeJson(doc_out, register_buf.data(), register_buf.size());

  websocket_client.sendTXT(register_buf.data(), n);
}

void WebSocket::sendError(const ErrorResult& error, const String& request_id) {
  TRACEF("origin: %s message: %s request_id: %s\n", error.who_.c_str(),
         error.detail_.c_str(), request_id.c_str());
  doc_out.clear();

  // Use ther error message type
  doc_out["type"] = "err";

  // Where the message originated from
  doc_out["context"] = error.who_.c_str();

  // The error itself
  doc_out["message"] = error.detail_.c_str();

  // The request ID to enable tracing
  doc_out["request_id"] = request_id.c_str();

  std::vector<char> register_buf = std::vector<char>(measureJson(doc_out) + 1);
  size_t n = serializeJson(doc_out, register_buf.data(), register_buf.size());

  websocket_client.sendTXT(register_buf.data(), n);
}

void WebSocket::sendDebug(const String& message) {
  TRACEF("message: %s\n", message.c_str());
  doc_out.clear();

  // Use ther error message type
  doc_out["type"] = "dbg";

  // The error itself
  doc_out["message"] = message.c_str();

  std::vector<char> register_buf = std::vector<char>(measureJson(doc_out) + 1);
  size_t n = serializeJson(doc_out, register_buf.data(), register_buf.size());

  websocket_client.sendTXT(register_buf.data(), n);
}

void WebSocket::sendResults(JsonObjectConst results) {
  std::vector<char> buffer = std::vector<char>(measureJson(results) + 1);
  size_t n = serializeJson(results, buffer.data(), buffer.size());
  TRACELN(buffer.data());
  websocket_client.sendTXT(buffer.data(), n);
}

void WebSocket::sendSystem(JsonObject data) {
  data[WebSocket::type_key_] = WebSocket::system_type_;

  std::vector<char> data_buf = std::vector<char>(measureJson(data) + 1);
  size_t n = serializeJson(data, data_buf.data(), data_buf.size());

  websocket_client.sendTXT(data_buf.data(), n);
}

const char* WebSocket::getRootCas() const {
  // If root_cas_ not loaded, use default root CA
  return root_cas_.length() ? root_cas_.c_str() : default_root_ca;
}

void WebSocket::setWsToken(const char* ws_token) {
  is_setup_ = false;
  ws_token_.clear();
  ws_token_.reserve(6 + strlen(ws_token));
  ws_token_ = F("token_");
  ws_token_ += ws_token;
  TRACEF("Set token: %s\n", ws_token_.c_str());
}

const bool WebSocket::isWsTokenSet() const { return !ws_token_.isEmpty(); }

void WebSocket::handleEvent(WStype_t type, uint8_t* payload, size_t length) {
  // Print class type before the printing the message type
  switch (type) {
    case WStype_DISCONNECTED: {
      TRACELN(F(": Disconnected!"));
      last_connect_up_ = std::chrono::steady_clock::now();
    } break;
    case WStype_CONNECTED: {
      TRACEF("Connected to: %s\n", reinterpret_cast<char*>(payload));
    } break;
    case WStype_TEXT: {
      TRACEF("Got text %u: %s\n", length, reinterpret_cast<char*>(payload));
      handleData(payload, length);
    } break;
    case WStype_PING:
      TRACELN(F("Received ping"));
      break;
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    case WStype_PONG:
      TRACEF("Unhandled message type: %u\n", type);
      break;
  }
}

void WebSocket::handleData(const uint8_t* payload, size_t length) {
  // Deserialize the JSON object into allocated memory
  doc_in.clear();
  const DeserializationError error = deserializeJson(doc_in, payload, length);
  if (error) {
    sendError(type(), String(F("Deserialize failed: ")) + error.c_str());
    return;
  }

  // Pass the message to the peripheral and task handlers
  peripheral_controller_callback_(doc_in.as<JsonObjectConst>());
  task_controller_callback_(doc_in.as<JsonObjectConst>());
  if (ota_update_callback_) {
    ota_update_callback_(doc_in.as<JsonObjectConst>());
  }
}

void WebSocket::updateUpDownTime(const bool is_connected) {
  if (is_connected != was_connected_) {
    was_connected_ = is_connected;
    const auto now = std::chrono::steady_clock::now();
    if (is_connected) {
      TRACELN("WS connected");
      // Connection went up
      if (last_connect_down_ != std::chrono::steady_clock::time_point::min()) {
        // Only update if last_connect_down_ has been set
        last_down_duration_ = now - last_connect_down_;
      }
      last_connect_up_ = now;
      send_on_connect_messages_ = true;
    } else {
      // Connection went down
      if (last_connect_up_ != std::chrono::steady_clock::time_point::min()) {
        // Only update if last_connect_up_ has been set
        last_up_duration_ = now - last_connect_up_;
      }
      last_connect_down_ = now;
    }
  }
}

void WebSocket::sendUpDownTimeData() {
  // Only send after valid times have been recorded
  if (last_up_duration_ != std::chrono::steady_clock::duration::min() &&
      last_down_duration_ != std::chrono::steady_clock::duration::min()) {
    doc_out.clear();
    if (last_up_duration_ != std::chrono::steady_clock::duration::min()) {
      // If the up duration is valid, print and send the data
      int64_t last_up_duration_s =
          std::chrono::duration_cast<std::chrono::seconds>(last_up_duration_)
              .count();
      TRACEF("Last WS up duration: %llds\n", last_up_duration_s);
      doc_out[F("last_ws_up_duration_s")] =
          std::chrono::duration_cast<std::chrono::seconds>(last_up_duration_)
              .count();
    }
    if (last_down_duration_ != std::chrono::steady_clock::duration::min()) {
      // If the down duration is valid, print and send the data
      int64_t last_down_duration_s =
          std::chrono::duration_cast<std::chrono::seconds>(last_down_duration_)
              .count();
      TRACEF("Last WS down duration: %llds\n", last_down_duration_s);
      doc_out[F("last_ws_down_duration_s")] =
          std::chrono::duration_cast<std::chrono::seconds>(last_down_duration_)
              .count();
    }
    sendSystem(doc_out.as<JsonObject>());
  }
}

void WebSocket::restartOnUnimplementedFunction() {
  TRACELN(F("Unimplemented Function"));
  delay(10000);
  abort();
}

const __FlashStringHelper* WebSocket::firmware_version_ =
    FPSTR(FIRMWARE_VERSION);

const char* WebSocket::core_domain_key_ = "core_domain";
const char* WebSocket::ws_token_key_ = "ws_token";
const char* WebSocket::secure_url_key_ = "secure_url";

const __FlashStringHelper* WebSocket::request_id_key_ = FPSTR("request_id");
const __FlashStringHelper* WebSocket::type_key_ = FPSTR("type");
const __FlashStringHelper* WebSocket::result_type_ = FPSTR("result");
const __FlashStringHelper* WebSocket::telemetry_type_ = FPSTR("tel");
const __FlashStringHelper* WebSocket::task_key_ = FPSTR("task_id");
const __FlashStringHelper* WebSocket::system_type_ = FPSTR("sys");

}  // namespace inamata
