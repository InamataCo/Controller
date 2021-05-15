#pragma once
#include <functional>

#include "managers/network.h"
#include "managers/server.h"

namespace bernd_box {
/**
 * Struct with callbacks to get dynamic system services
 */
struct ServiceGetters {
  std::function<std::shared_ptr<Network>()> get_network;
  std::function<std::shared_ptr<Server>()> get_server;

  static const __FlashStringHelper* nullptr_error_;
  static const __FlashStringHelper* network_nullptr_error_;
  static const __FlashStringHelper* server_nullptr_error_;
};
}  // namespace bernd_box