#pragma once

#include <WiFiMulti.h>

#include <chrono>

#include "managers/types.h"

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
   * 
   * \param acces_points The WiFi access points to try to connect to
   */
  Network(std::vector<WiFiAP>& access_points);

  /**
   * Connects to the configured WiFi access point and contacts the SDG server
   *
   * Blocks until the connection has been made.
   *
   * \param timeout The length of time to wait until aborting
   * \return True if successful
   */
  bool connect(std::chrono::duration<int> timeout);

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
  std::vector<WiFiAP> wifi_aps_;
};

}  // namespace bernd_box
