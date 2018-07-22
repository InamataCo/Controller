/**
 * All user-configurations are set here
 *
 * This includes Wifi's SSID and password.
 *
 * \author Moritz Ulmer <moritz.ulmer@posteo.de>
 * \date 2018
 * \copyright Apache License 2.0
 */

#ifndef BERND_BOX_CONFIGURATION_H
#define BERND_BOX_CONFIGURATION_H

#include <chrono>

namespace bernd_box {

// WiFi
const char* ssid = "your-ssid";
const char* password = "your-password";
const std::chrono::seconds wifi_connect_timeout(20);

// MQTT
const char* mqtt_server = "192.168.0.1";
const char* client_id = "fiona_box1";

}  // namespace bernd_box

#endif