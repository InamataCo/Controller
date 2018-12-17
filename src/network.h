/**
 * Wifi related functionality
 *
 * Functionality includes conencting to wifi
 *
 * \author Moritz Ulmer <moritz.ulmer@posteo.de>
 * \date 2018
 * \copyright Apache License 2.0
 */

#ifndef BERND_BOX_NETWORK_H
#define BERND_BOX_NETWORK_H

#include <WiFi.h>
#include <chrono>

namespace bernd_box {
class Network {
 public:
  /**
   * WiFi helper class that deals with connection time-outs and checking its
   * state
   */
  Network(const char* ssid, const char* password)
      : ssid_(ssid),
        password_(password),
        connect_wait_duration_(std::chrono::milliseconds(500)) {}

  /**
   * Connects to the configured WiFi access point
   *
   * Blocks until the connection has been made
   *
   * \param timeout The length of time to wait until aborting
   * \return True if successful
   */
  bool connect(std::chrono::duration<int> timeout) {
    Serial.printf("WiFi: Attempting to connect to %s\n", ssid_);

    WiFi.begin(ssid_, password_);

    std::chrono::milliseconds total_wait_time(0);

    // Attempt to connect until timeout is reached
    while (WiFi.status() != WL_CONNECTED && total_wait_time < timeout) {
      delay(std::chrono::duration_cast<std::chrono::milliseconds>(
                connect_wait_duration_)
                .count());
      total_wait_time += connect_wait_duration_;
      Serial.print(".");
    }
    Serial.println();

    return isConnected();
  }

  /**
   * Prints the current WiFi state to the serial terminal
   */
  void printState() {
    Serial.printf("Connected: %d\n", isConnected());
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
  }

  String getSsid() { return WiFi.SSID(); }

  /**
   * Checks whether the ESP is connected to WiFi
   *
   * \return isConnected True if connected
   */
  bool isConnected() { return WiFi.status() == WL_CONNECTED; }

 private:
  const char* ssid_;
  const char* password_;

  std::chrono::milliseconds connect_wait_duration_;
};

}  // namespace bernd_box

#endif