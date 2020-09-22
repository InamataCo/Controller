#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include <chrono>
#include <initializer_list>

#include "managers/types.h"
#include "utils/setupNode.h"

namespace bernd_box {

/**
 * Wifi related functionality
 *
 * Functionality includes conencting to wifi
 */
class Network {
 public:
  /**
   * WiFi helper class that deals with connection time-outs and checking its
   * state
   */
  Network(std::initializer_list<AccessPoint>& access_points,
          const char* core_domain, const char* root_cas = nullptr);

  /**
   * Connects to the configured WiFi access point and contacts the SDG server
   *
   * Blocks until the connection has been made.
   *
   * \param timeout The length of time to wait until aborting
   * \return True if successful
   */
  bool connect(std::chrono::duration<int> timeout);

  String pingSdgServer();

  /**
   * Asks the SDG server for the IP address of the local coordinator.
   *
   * The coordinator hosts the MQTT broker, which is required for local
   * communication.
   *
   * \return IP address of the local MQTT server, empty if not found
   */
  String getCoordinatorLocalIpAddress();

  /**
   * Prints the current WiFi state to the serial terminal
   */
  void printState();

  // Not sure if WiFiClientSecure checks the validity date of the certificate.
  // Setting clock just to be sure...
  int setClock(std::chrono::seconds timeout_s);

  bool isTimeSet();

  String getSsid();

  /**
   * Checks whether the ESP is connected to WiFi
   *
   * \return isConnected True if connected
   */
  bool isConnected();

 private:
  WiFiMulti wiFiMulti_;
  std::initializer_list<AccessPoint>& access_points_;

  /// HTTPS client with support for TLS connections
  HTTPClient httpClient_;

  std::chrono::milliseconds connect_wait_duration_;

  // SDG Server
  /// URL to for controllers to ping the server
  String ping_url_;
  const char* core_domain_;
  const char* ping_path_ = "/api/v1/farms/controllers/ping/";

  /// Currently the Let's Encrypt staging root certificate authority (CA)
  const char* root_cas_;
};

}  // namespace bernd_box
