#include "web_socket.h"

#include <esp_tls.h>

namespace bernd_box {

WebSocket::WebSocket(
    std::function<std::vector<utils::UUID>()> get_peripheral_ids,
    Server::Callback peripheral_controller_callback,
    std::function<std::vector<utils::UUID>()> get_task_ids,
    Server::Callback task_controller_callback, const char* core_domain,
    const char* ws_token, const char* root_cas)
    : get_peripheral_ids_(get_peripheral_ids),
      peripheral_controller_callback_(peripheral_controller_callback),
      get_task_ids_(get_task_ids),
      task_controller_callback_(task_controller_callback),
      core_domain_(core_domain),
      root_cas_(root_cas) {
  ws_token_ = F("token_");
  ws_token_ += ws_token;
  esp_tls_init_global_ca_store();
  esp_tls_set_global_ca_store((const unsigned char*) root_cas_.c_str(), root_cas_.length());
}

const String& WebSocket::type() {
  static const String name{"WebSocket"};
  return name;
}

bool WebSocket::isConnected() { return WebSocketsClient::isConnected(); }

bool WebSocket::connect(std::chrono::seconds timeout) {
  // Configure the WebSocket interface with the server, TLS certificate and the
  // reconnect interval
  if (!is_setup_) {
    is_setup_ = true;
    if (root_cas_.length()) {
      beginSslWithCA(core_domain_.c_str(), 443, controller_path_,
                     root_cas_.c_str(), ws_token_.c_str());
    } else {
      begin(core_domain_.c_str(), 8000, controller_path_, ws_token_.c_str());
    }
    onEvent(std::bind(&WebSocket::handleEvent, this, _1, _2, _3));
    setReconnectInterval(5000);
  }

  // Attempt to connect to the server until connected or it times out
  std::chrono::milliseconds connect_start(millis());
  while (!isConnected()) {
    if (std::chrono::milliseconds(millis()) - connect_start > timeout) {
      return false;
    }
    loop();
  }

  return true;
}

void WebSocket::handle() { loop(); }

void WebSocket::send(const String& name, double value) {
  Serial.print(F("Unimplemented Function: "));
  Serial.println(F(__PRETTY_FUNCTION__));
  delay(10000);
  ESP.restart();
}
void WebSocket::send(const String& name, int value) {
  Serial.print(F("Unimplemented Function: "));
  Serial.println(F(__PRETTY_FUNCTION__));
  delay(10000);
  ESP.restart();
}
void WebSocket::send(const String& name, bool value) {
  Serial.print(F("Unimplemented Function: "));
  Serial.println(F(__PRETTY_FUNCTION__));
  delay(10000);
  ESP.restart();
}

void WebSocket::send(const String& name, DynamicJsonDocument& doc) {
  doc["type"] = "tel";
  doc["name"] = name;

  // Calculate the size of the resultant serialized JSON, create a buffer of
  // that size and serialize the JSON into that buffer.
  // Add extra byte for the null terminator
  std::vector<char> register_buf = std::vector<char>(measureJson(doc) + 1);
  size_t n = serializeJson(doc, register_buf.data(), register_buf.size());

  sendTXT(register_buf.data(), n);
}

void WebSocket::send(const String& name, const char* value, size_t length) {
  Serial.print(F("Unimplemented Function: "));
  Serial.println(F(__PRETTY_FUNCTION__));
  delay(10000);
  ESP.restart();
}

void WebSocket::sendTelemetry(const utils::UUID& task_id, JsonObject data) {
  data[Server::type_key_] = Server::telemetry_type_;
  data[Server::task_id_key_] = task_id.toString();

  std::vector<char> register_buf = std::vector<char>(measureJson(data) + 1);
  size_t n = serializeJson(data, register_buf.data(), register_buf.size());
  Serial.printf("Telemetry size: %i\n", n);

  sendTXT(register_buf.data(), n);
}

void WebSocket::sendRegister() {
  DynamicJsonDocument doc(BB_JSON_PAYLOAD_SIZE);

  // Use ther register message type
  doc["type"] = "reg";

  // Collect all added peripheral ids and write them to a JSON doc
  std::vector<utils::UUID> peripheral_ids = get_peripheral_ids_();
  if (!peripheral_ids.empty()) {
    JsonArray peripherals = doc.createNestedArray(F("peripherals"));
    for (const auto& peripheral_id : peripheral_ids) {
      peripherals.add(peripheral_id.toString());
    }
  }

  // Collect all running task ids and write them to a JSON doc
  std::vector<utils::UUID> task_ids = get_task_ids_();
  if (!task_ids.empty()) {
    JsonArray tasks = doc.createNestedArray(F("tasks"));
    for (const auto& task_id : task_ids) {
      tasks.add(task_id.toString());
    }
  }

  // Calculate the size of the resultant serialized JSON, create a buffer of
  // that size and serialize the JSON into that buffer.
  // Add extra byte for the null terminator
  std::vector<char> register_buf = std::vector<char>(measureJson(doc) + 1);
  size_t n = serializeJson(doc, register_buf.data(), register_buf.size());

  sendTXT(register_buf.data(), n);
}

void WebSocket::sendError(const String& who, const String& message) {
  Serial.println(message);

  DynamicJsonDocument doc(BB_JSON_PAYLOAD_SIZE);

  // Use ther error message type
  doc["type"] = "err";

  // Place the error message
  doc["message"] = message.c_str();

  std::vector<char> register_buf = std::vector<char>(measureJson(doc) + 1);
  size_t n = serializeJson(doc, register_buf.data(), register_buf.size());

  sendTXT(register_buf.data(), n);
}

void WebSocket::sendError(const ErrorResult& error, const String& request_id) {
  Serial.printf("origin: %s message: %s request_id: %s\n", error.who_.c_str(),
                error.detail_.c_str(), request_id.c_str());

  DynamicJsonDocument doc(BB_JSON_PAYLOAD_SIZE);

  // Use ther error message type
  doc["type"] = "err";

  // Where the message originated from
  doc["context"] = error.who_.c_str();

  // The error itself
  doc["message"] = error.detail_.c_str();

  // The request ID to enable tracing
  doc["request_id"] = request_id.c_str();

  std::vector<char> register_buf = std::vector<char>(measureJson(doc) + 1);
  size_t n = serializeJson(doc, register_buf.data(), register_buf.size());

  sendTXT(register_buf.data(), n);
}

void WebSocket::sendResults(JsonObjectConst results) {
  std::vector<char> buffer = std::vector<char>(measureJson(results) + 1);
  size_t n = serializeJson(results, buffer.data(), buffer.size());

  sendTXT(buffer.data(), n);
}

void WebSocket::sendSystem(JsonObject data) {
  data[Server::type_key_] = Server::system_type_;

  std::vector<char> data_buf = std::vector<char>(measureJson(data) + 1);
  size_t n = serializeJson(data, data_buf.data(), data_buf.size());

  sendTXT(data_buf.data(), n);
}

void WebSocket::handleEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED: {
      _lastConnectionFail = millis();
      Serial.printf("WebSocket::HandleEvent: Disconnected!\n");
    } break;
    case WStype_CONNECTED: {
      Serial.printf("WebSocket::HandleEvent: Connected to url: %s\n", payload);
    } break;
    case WStype_TEXT: {
      Serial.printf("WebSocket::HandleEvent: get text: %s\n", payload);
      handleData(payload, length);
    } break;
    case WStype_BIN: {
      Serial.printf("WebSocket::HandleEvent: get binary length: %u\n", length);
      hexdump(payload, length);

      // send data to server
      // webSocket.sendBIN(payload, length);
    } break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    case WStype_PING:
    case WStype_PONG:
      break;
  }
}

void WebSocket::handleData(const uint8_t* payload, size_t length) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  // Deserialize the JSON object into allocated memory
  DynamicJsonDocument doc(BB_JSON_PAYLOAD_SIZE);
  const DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    sendError(who, String(F("Deserialize failed: ")) + error.c_str());
    return;
  }

  // Pass the message to the peripheral and task handlers
  peripheral_controller_callback_(doc.as<JsonObjectConst>());
  task_controller_callback_(doc.as<JsonObjectConst>());
}

void WebSocket::hexdump(const void* mem, uint32_t len, uint8_t cols) {
  const uint8_t* src = (const uint8_t*)mem;
  Serial.printf("\nWebSocket::Hexdump: Address: 0x%08X len: 0x%X (%d)",
                (ptrdiff_t)src, len, len);
  for (uint32_t i = 0; i < len; i++) {
    if (i % cols == 0) {
      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }
    Serial.printf("%02X ", *src);
    src++;
  }
  Serial.printf("\n");
}

}  // namespace bernd_box
