#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include <chrono>

#include "config.h"
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
  Network(const char* ssid, const char* password);

  /**
   * Connects to the configured WiFi access point and contacts the SDG server
   *
   * Blocks until the connection has been made. Also sends a JSON to the SDG
   * server containing its UUID and MAC address.
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
  const char* ssid_;
  const char* password_;

  WiFiMulti wiFiMulti_;

  /// HTTPS client with support for TLS connections
  HTTPClient httpClient_;

  std::chrono::milliseconds connect_wait_duration_;

  // SDG Server
  /// URL to for controllers to ping the server
  const char* ping_url_ =
      "https://core.staging.openfarming.ai/api/v1/farms/controllers/ping/";
  /// Currently the Let's Encrypt staging root certificate authority (CA)
  const char* root_ca_ =
      "-----BEGIN CERTIFICATE-----\n"
      "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\n"
      "-----END CERTIFICATE-----\n";
};

}  // namespace bernd_box
