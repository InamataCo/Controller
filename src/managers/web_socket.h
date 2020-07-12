#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>

#include <map>

#include "server.h"

namespace bernd_box {

using namespace std::placeholders;

class WebSocket : public Server, private WebSocketsClient {
 public:
  /**
   * Connection to the SDG server over websockets.
   *
   * This enables bi-directional communication between the controller and the
   * server while removing the intermediate such as the Coordinator over MQTT.
   */
  WebSocket(std::function<std::vector<String>()> get_factory_names,
            Callback object_callback, Callback task_callback);

  bool isConnected() final;
  bool connect() final;

  void handle() final;

  void send(const String& name, double value) final;
  void send(const String& name, int value) final;
  void send(const String& name, bool value) final;
  void send(const String& name, DynamicJsonDocument& doc) final;
  void send(const String& name, const char* value, size_t length) final;

  void sendRegister() final;
  void sendError(const String& who, const String& message) final;
  void addAction(const String& name, Callback callback) final;
  void removeAction(const String& topic) final;
  const CallbackMap& getCallbackMap() final;

 private:
  void handleEvent(WStype_t type, uint8_t* payload, size_t length);
  void hexdump(const void* mem, uint32_t len, uint8_t cols = 16);

  bool is_setup_ = false;

  std::function<std::vector<String>()> get_factory_names_;
  Callback object_callback_;
  Callback task_callback_;

  static const char* dst_ca_;
};

}  // namespace bernd_box