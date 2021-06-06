/**
 * All user-configurations are set here
 *
 * This includes Wifi's SSID and password.
 */

#pragma once

#include <chrono>
#include <initializer_list>

#include "managers/types.h"

namespace bernd_box {

// Connectivity
extern const std::chrono::milliseconds check_connectivity_period;

// WiFi
extern const std::chrono::seconds wifi_connect_timeout;

// Server certificate authorities TLS certificates
extern const std::chrono::seconds server_connect_timeout;


}  // namespace bernd_box
