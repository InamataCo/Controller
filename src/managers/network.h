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
    kConnected,
    kFastConnect,
    kScanning,
    kMultiConnect,
    kHiddenConnect,
    kCyclePower
  };

  /**
   * WiFi helper class that deals with connection time-outs and checking its
   * state
   *
   * \param acces_points The WiFi access points to try to connect to
   */
  Network(std::vector<WiFiAP>& access_points, String& controller_name);

  void setMode(ConnectMode mode);

  /**
   * Connects to the configured WiFi access point
   *
   * Expects to be called periodically to handle the WiFi connection procedure.
   * Does not block, and will cycle through, AP search, connect to known and
   * then unknown APs, before power cycling the modem.
   *
   * \return True if connected
   */
  ConnectMode connect();

  /**
   * Checks if connected to a WiFi network
   *
   * \param wifi_status Use output of WiFi.status() (Optional)
   * \return True if connected
   */
  bool isConnected(wl_status_t* wifi_status = nullptr);

  // Not sure if WiFiClientSecure checks the validity date of the certificate.
  // Setting clock just to be sure...
  int setClock(std::chrono::seconds timeout_s);

  static bool populateNetworkInfo(NetworkInfo& network_info);

  /**
   * Sorts WiFi networks by descending signal strength (unknown to back)
   *
   * @return true if rhs has weaker signal or unknown internal ID
   * @return false if lhs has weaker signal or unknown internal ID
   */
  static bool sortRssi(const WiFiAP& lhs, const WiFiAP& rhs);

  std::vector<WiFiAP> wifi_aps_;

 private:
  /**
   * Perform WiFi fast connect attempt (non-blocking)
   *
   * @return True if connected
   */
  bool tryFastConnect();

  /**
   * Perform WiFi AP scan attempt (non-blocking)
   *
   * @return Always false
   */
  bool tryScanning();

  /**
   * Perform WiFi connection attempt (non-blocking)
   *
   * @return True if connected
   */
  bool tryMultiConnect();

  /**
   * Perform WiFi connection attempt to hidden AP (non-blocking)
   *
   * @return True if connected
   */
  bool tryHiddenConnect();

  /**
   * Perform WiFi modem power cycling (non-blocking)
   *
   * @return Always false
   */
  bool tryCyclePower();

  std::vector<WiFiAP>::iterator current_wifi_ap_;
  String controller_name_;

  ConnectMode connect_mode_ = ConnectMode::kFastConnect;
  std::chrono::steady_clock::time_point connect_start_ =
      std::chrono::steady_clock::time_point::min();
  std::chrono::milliseconds connect_timeout_ = std::chrono::seconds(20);

  std::chrono::steady_clock::time_point scan_start_ =
      std::chrono::steady_clock::time_point::min();
  std::chrono::milliseconds scan_timeout_ = std::chrono::seconds(30);

  bool multi_connect_first_run_ = true;
};

}  // namespace inamata
