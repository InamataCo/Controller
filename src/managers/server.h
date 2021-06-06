#pragma once

#include <functional>
#include <map>

#include "managers/io_types.h"
#include "utils/uuid.h"

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

  virtual bool connect(std::chrono::seconds timeout) = 0;
  virtual bool isConnected() = 0;

  virtual void handle() = 0;

  virtual void send(const String& name, double value) = 0;
  virtual void send(const String& name, int value) = 0;
  virtual void send(const String& name, bool value) = 0;
  virtual void send(const String& name, DynamicJsonDocument& doc) = 0;
  virtual void send(const String& name, const char* value, size_t length) = 0;

  virtual void sendTelemetry(const utils::UUID& uuid, JsonObject data) = 0;
  virtual void sendRegister() = 0;
  virtual void sendError(const String& who, const String& message) = 0;
  virtual void sendError(const ErrorResult& error,
                         const String& request_id = "") = 0;

  virtual void sendResults(JsonObjectConst results) = 0;
  virtual void sendSystem(JsonObject data) = 0;

  virtual const String& getRootCas() const = 0;

  static const __FlashStringHelper* request_id_key_;
  static const __FlashStringHelper* type_key_;
  static const __FlashStringHelper* result_type_;
  static const __FlashStringHelper* telemetry_type_;
  static const __FlashStringHelper* task_id_key_;
  static const __FlashStringHelper* system_type_;
};
}  // namespace bernd_box
