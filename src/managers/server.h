#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include <functional>
#include <map>

#include "managers/io_types.h"

namespace bernd_box {

/**
 * Interface class for connections to the server
 *
 * Defines the functions that are used to connect to, send data to and handle
 * messages coming from the server.
 */
class Server {
 public:
  using Callback = std::function<void(const JsonObjectConst& message)>;
  using CallbackMap = std::map<String, Callback>;

  virtual ~Server() = default;

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
  virtual void sendError(const ErrorResult& error, const String& request_id = "") = 0;
};
}  // namespace bernd_box
