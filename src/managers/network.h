#pragma once

#include <WiFiManager.h>

#include <chrono>
#include <vector>

#include "managers/types.h"

namespace inamata {

struct NetworkInfo {
  int16_t id;
  String ssid;
  int32_t rssi;
  uint8_t encType;
  uint8_t* bssid;
  int32_t channel;
  #ifndef ESP32
  bool hidden;
  #endif
};

/**
 * Wifi related functionality
 *
 * Functionality includes conencting to wifi
 */
class Network {
 public:
  enum class ConnectMode {
    kFastConnect,
    kScanning,
    kMultiConnect,
    kHiddenConnect
  };

  /**
   * WiFi helper class that deals with connection time-outs and checking its
   * state
   *
   * \param acces_points The WiFi access points to try to connect to
   */
  Network(std::vector<WiFiAP>& access_points, String& controller_name);

  /**
   * Connects to the configured WiFi access point
   *
   * \return True if connected
   */
  bool connect();

  /**
   * Prints the current WiFi state to the serial terminal
   */
  void printState();

  // Not sure if WiFiClientSecure checks the validity date of the certificate.
  // Setting clock just to be sure...
  int setClock(std::chrono::seconds timeout_s);

  bool isTimeSet();

  String getSsid();

  static bool populateNetworkInfo(NetworkInfo& network_info);

  /**
   * Sorts WiFi networks by descending signal strength (unknown to back)
   *
   * @return true if rhs has weaker signal or unknown internal ID
   * @return false if lhs has weaker signal or unknown internal ID
   */
  static bool sortRssi(const WiFiAP& lhs, const WiFiAP& rhs);

 private:
  std::vector<WiFiAP> wifi_aps_;
  std::vector<WiFiAP>::iterator current_wifi_ap_;
  String controller_name_;

  ConnectMode connect_mode_ = ConnectMode::kFastConnect;
  std::chrono::steady_clock::time_point connect_start_ =
      std::chrono::steady_clock::time_point::min();
  std::chrono::milliseconds connect_timeout_ = std::chrono::seconds(5);

  std::chrono::steady_clock::time_point scan_start_ =
      std::chrono::steady_clock::time_point::min();
  std::chrono::milliseconds scan_timeout_ = std::chrono::seconds(5);

  bool multi_connect_first_run_ = true;
};

}  // namespace inamata
