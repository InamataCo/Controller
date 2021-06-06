#pragma once
#include <functional>

#include "managers/network.h"
#include "managers/server.h"

namespace bernd_box {
/**
 * Struct with callbacks to get dynamic system services
 */
struct ServiceGetters {
  ServiceGetters() = default;
  ServiceGetters(std::function<std::shared_ptr<Network>()> get_network,
                 std::function<std::shared_ptr<Server>()> get_server)
      : getNetwork(get_network), getServer(get_server) {}

  std::function<std::shared_ptr<Network>()> getNetwork = []() {
    return nullptr;
  };
  std::function<std::shared_ptr<Server>()> getServer = []() { return nullptr; };

  static const __FlashStringHelper* network_nullptr_error_;
  static const __FlashStringHelper* server_nullptr_error_;
};
}  // namespace bernd_box