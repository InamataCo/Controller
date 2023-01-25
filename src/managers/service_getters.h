#pragma once
#include <functional>

#include "managers/network.h"
#include "managers/storage.h"
#include "managers/web_socket.h"

namespace inamata {
/**
 * Struct with callbacks to get dynamic system services
 */
struct ServiceGetters {
  ServiceGetters() = default;
  ServiceGetters(std::function<std::shared_ptr<Network>()> get_network,
                 std::function<std::shared_ptr<WebSocket>()> get_web_socket,
                 std::function<std::shared_ptr<Storage>()> get_storage)
      : getNetwork(get_network),
        getWebSocket(get_web_socket),
        getStorage(get_storage) {}

  std::function<std::shared_ptr<Network>()> getNetwork = []() {
    return nullptr;
  };
  std::function<std::shared_ptr<WebSocket>()> getWebSocket = []() {
    return nullptr;
  };
  std::function<std::shared_ptr<Storage>()> getStorage = []() {
    return nullptr;
  };

  static const __FlashStringHelper* network_nullptr_error_;
  static const __FlashStringHelper* web_socket_nullptr_error_;
  static const __FlashStringHelper* storage_nullptr_error_;
};
}  // namespace inamata