/**
 * Wifi related functionality
 *
 * Functionality includes conencting to wifi
 *
 * \author Moritz Ulmer <moritz.ulmer@posteo.de>
 * \date 2018
 * \copyright Apache License 2.0
 */

#ifndef BERND_BOX_WIFI_H
#define BERND_BOX_WIFI_H

#include <WiFi.h>
#include <chrono>

namespace bernd_box {

class Wifi {
 public:
  Wifi(const char* ssid, const char* password)
      : ssid_(ssid),
        password_(password),
        connect_wait_duration_(std::chrono::milliseconds(500)) {}

  /**
   * Connects to the configured WiFi access point
   *
   * Blocks until the connection has been made
   *
   * @param timeout The length of time to wait until aborting
   * @return True if successful
   */
  bool connect(std::chrono::duration<int> timeout) {
    Serial.print("Connecting to ");
    Serial.println(ssid_);

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

    // Check whether connection to WiFi was successful
    bool connected;
    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
    } else {
      connected = false;
    }
    return connected;
  }

  void printState() {
    Serial.print("IP Address ");
    Serial.println(WiFi.localIP());
  }

  bool isConnected() { return WiFi.status() == WL_CONNECTED; }

 private:
  const char* ssid_;
  const char* password_;

  std::chrono::milliseconds connect_wait_duration_;
};

}  // namespace bernd_box

#endif