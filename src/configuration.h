/**
 * All user-configurations are set here
 *
 * This includes Wifi's SSID and password.
 *
 * \author Moritz Ulmer <moritz.ulmer@posteo.de>
 * \date 2018
 * \copyright Apache License 2.0
 */

#pragma once

#include <chrono>
#include <initializer_list>

#include "managers/types.h"

namespace bernd_box {

// Connectivity
extern const std::chrono::milliseconds check_connectivity_period;

// WiFi
extern std::initializer_list<AccessPoint> access_points;
extern const std::chrono::seconds wifi_connect_timeout;

// MQTT
extern const char* client_id;
extern const uint mqtt_connection_attempts;  // Maximum attempts before aborting

// Server certificate authorities TLS certificates
extern const std::chrono::seconds server_connect_timeout;
extern const char* core_domain;
extern const char* ws_token;
extern const char* root_cas;

}  // namespace bernd_box
