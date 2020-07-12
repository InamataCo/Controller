#include "web_socket.h"

namespace bernd_box {

WebSocket::WebSocket(
    std::function<std::vector<String>()> get_factory_names,
    std::function<void(char*, uint8_t*, unsigned int)> object_callback,
    std::function<void(char*, uint8_t*, unsigned int)> task_callback)
    : get_factory_names_(get_factory_names),
      object_callback_(object_callback),
      task_callback_(task_callback) {}

bool WebSocket::isConnected() { return WebSocketsClient::isConnected(); }

bool WebSocket::connect() {
  if (!is_setup_) {

    beginSslWithCA("echo.websocket.org", 443, "/", dst_ca_);
    onEvent(std::bind(&WebSocket::handleEvent, this, _1, _2, _3));
    setReconnectInterval(5000);

    while (!isConnected()) {
      loop();
    }

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
  Serial.print(F("Unimplemented Function: "));
  Serial.println(F(__PRETTY_FUNCTION__));
  delay(10000);
  ESP.restart();
}
void WebSocket::sendError(const String& who, const String& message) {
  Serial.print(F("Unimplemented Function: "));
  Serial.println(F(__PRETTY_FUNCTION__));
  delay(10000);
  ESP.restart();
}
void WebSocket::addAction(const String& name, Callback callback) {
  Serial.print(F("Unimplemented Function: "));
  Serial.println(F(__PRETTY_FUNCTION__));
  delay(10000);
  ESP.restart();
}
void WebSocket::removeAction(const String& topic) {
  Serial.print(F("Unimplemented Function: "));
  Serial.println(F(__PRETTY_FUNCTION__));
  delay(10000);
  ESP.restart();
}
const WebSocket::CallbackMap& WebSocket::getCallbackMap() {
  Serial.print(F("Unimplemented Function: "));
  Serial.println(F(__PRETTY_FUNCTION__));
  delay(10000);
  ESP.restart();
}

void WebSocket::handleEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("WebSocket::HandleEvent: Disconnected!\n");
      break;
    case WStype_CONNECTED: {
      Serial.printf("WebSocket::HandleEvent: Connected to url: %s\n", payload);

      // send message to server when Connected
      sendTXT("WebSocket::HandleEvent: Connected");
    } break;
    case WStype_TEXT:
      Serial.printf("WebSocket::HandleEvent: get text: %s\n", payload);

      // send message to server
      // webSocket.sendTXT("message here");
      break;
    case WStype_BIN:
      Serial.printf("WebSocket::HandleEvent: get binary length: %u\n", length);
      hexdump(payload, length);

      // send data to server
      // webSocket.sendBIN(payload, length);
      break;
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

const char* WebSocket::dst_ca_ = \
     "-----BEGIN CERTIFICATE-----\n" \
     "MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n" \
     "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
     "DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n" \
     "SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n" \
     "GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n" \
     "AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n" \
     "q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n" \
     "SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n" \
     "Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n" \
     "a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n" \
     "/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n" \
     "AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n" \
     "CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n" \
     "bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n" \
     "c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n" \
     "VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n" \
     "ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n" \
     "MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n" \
     "Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n" \
     "AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n" \
     "uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n" \
     "wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n" \
     "X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n" \
     "PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n" \
     "KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n" \
     "-----END CERTIFICATE-----\n";

}  // namespace bernd_box
