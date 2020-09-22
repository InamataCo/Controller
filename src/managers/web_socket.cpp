#include "web_socket.h"

namespace bernd_box {

WebSocket::WebSocket(std::function<std::vector<String>()> get_peripheral_names,
                     Server::Callback peripheral_controller_callback,
                     std::function<std::vector<String>()> get_task_names,
                     Server::Callback task_controller_callback,
                     const char* core_domain,
                     const char* ws_token,
                     const char* root_cas)
    : get_peripheral_names_(get_peripheral_names),
      peripheral_controller_callback_(peripheral_controller_callback),
      get_task_names_(get_task_names),
      task_controller_callback_(task_controller_callback),
      core_domain_(core_domain),
      ws_token_(ws_token),
      root_cas_(root_cas) {}

const String& WebSocket::type() {
  static const String name{"WebSocket"};
  return name;
}

bool WebSocket::isConnected() { return WebSocketsClient::isConnected(); }

bool WebSocket::connect() {
  if (!is_setup_) {
    if(root_cas_) {
      beginSslWithCA(core_domain_, 443, controller_path_, root_cas_, ws_token_);
    } else {
      begin(core_domain_, 8000, controller_path_, ws_token_);
    }
    onEvent(std::bind(&WebSocket::handleEvent, this, _1, _2, _3));
    setReconnectInterval(5000);
  }

  // TODO: Add timeout to reboot ESP
  while (!isConnected()) {
    loop();
  }

  if (!is_setup_) {
    sendRegister();

    is_setup_ = true;
  }

  return isConnected();
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

void WebSocket::sendRegister() {
  DynamicJsonDocument doc(BB_JSON_PAYLOAD_SIZE);

  // Use ther register message type
  doc["type"] = "reg";

  // Collect all peripheral factory names and write them to a JSON doc
  std::vector<String> peripheral_names = get_peripheral_names_();
  DynamicJsonDocument peripherals_doc(JSON_ARRAY_SIZE(peripheral_names.size()));
  JsonArray peripheral_array = peripherals_doc.to<JsonArray>();
  Serial.println(F("Peripheral types:"));

  for (const auto& name : peripheral_names) {
    peripheral_array.add(name.c_str());
    Serial.printf("\t%s\n", name.c_str());
  }
  doc["peripheral_names"] = peripheral_array;

  // Collect all task factory names and write them to a JSON doc
  std::vector<String> task_names = get_task_names_();
  DynamicJsonDocument tasks_doc(JSON_ARRAY_SIZE(task_names.size()));
  JsonArray task_array = tasks_doc.to<JsonArray>();
  Serial.println(F("Task types:"));

  for (const auto& name : task_names) {
    task_array.add(name.c_str());
    Serial.printf("\t%s\n", name.c_str());
  }
  doc["task_names"] = task_array;

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
void WebSocket::sendError(const ErrorResult& error, const String& trace_id) {
  Serial.print("who: ");
  Serial.println(error.who_);
  Serial.print("detail: ");
  Serial.println(error.detail_);

  sendError(error.who_, error.detail_);
}

void WebSocket::handleEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED: {
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
