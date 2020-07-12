#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include <functional>
#include <map>

namespace bernd_box {
class Server {
 public:
  using Callback = std::function<void(char*, uint8_t*, unsigned int)>;
  using CallbackMap = std::map<String, Callback>;

  virtual bool Connect() = 0;
  virtual bool IsConnected() = 0;

  virtual void Loop() = 0;

  virtual void Send(const String& name, double value) = 0;
  virtual void Send(const String& name, int value) = 0;
  virtual void Send(const String& name, bool value) = 0;
  virtual void Send(const String& name, DynamicJsonDocument& doc) = 0;
  virtual void Send(const String& name, const char* value, size_t length) = 0;

  virtual void SendRegister() = 0;
  virtual void SendError(const String& who, const String& message) = 0;
  virtual void AddAction(const String& name, Callback callback) = 0;
  virtual void RemoveAction(const String& topic) = 0;
  virtual const CallbackMap& GetCallbackMap() = 0;
};
}  // namespace bernd_box
