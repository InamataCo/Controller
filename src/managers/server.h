#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include <functional>
#include <map>

namespace bernd_box {

/**
 * Interface class for connections to the server
 *
 * Defines the functions that are used to connect to, send data to and handle
 * messages coming from the server.
 */
class Server {
 public:
  using Callback = std::function<void(char*, uint8_t*, unsigned int)>;
  using CallbackMap = std::map<String, Callback>;

  virtual bool connect() = 0;
  virtual bool isConnected() = 0;

  virtual void handle() = 0;

  virtual void send(const String& name, double value) = 0;
  virtual void send(const String& name, int value) = 0;
  virtual void send(const String& name, bool value) = 0;
  virtual void send(const String& name, DynamicJsonDocument& doc) = 0;
  virtual void send(const String& name, const char* value, size_t length) = 0;

  virtual void sendRegister() = 0;
  virtual void sendError(const String& who, const String& message) = 0;
  virtual void addAction(const String& name, Callback callback) = 0;
  virtual void removeAction(const String& topic) = 0;
  virtual const CallbackMap& getCallbackMap() = 0;
};
}  // namespace bernd_box
