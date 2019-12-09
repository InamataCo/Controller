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
// const char* ssid = "SDGintern";
// const char* password = "8037473183859244";
const char* ssid = "PROTOHAUS";
const char* password = "PH-Wlan-2016#";
const std::chrono::seconds wifi_connect_timeout(20);

// MQTT
const char* mqtt_server = "192.168.0.15";
const char* client_id = "bernd_box_1";
const uint mqtt_connection_attempts = 3;  // Maximum attempts before aborting

}  // namespace bernd_box

#endif
