#include "network.h"

namespace inamata {

Network::Network(std::vector<WiFiAP>& wifi_aps, String& controller_name)
    : wifi_aps_(std::move(wifi_aps)),
      controller_name_(std::move(controller_name)) {
#ifdef ENABLE_TRACE
  TRACELN(F("Searching for APs:"));
  for (const WiFiAP& wifi_ap : wifi_aps_) {
    TRACEF("\t%s\n", wifi_ap.ssid.c_str());
  }
#endif
}

void Network::setMode(ConnectMode mode) { connect_mode_ = mode; }

Network::ConnectMode Network::connect() {
  // If not connected, execute active conntect mode. After trying action, try
  // check if subsequent action can be run
  bool connected = isConnected();
  if (!connected && connect_mode_ == ConnectMode::kFastConnect) {
    connected = tryFastConnect();
  }
  if (!connected && connect_mode_ == ConnectMode::kScanning) {
    connected = tryScanning();
  }
  if (!connected && connect_mode_ == ConnectMode::kMultiConnect) {
    connected = tryMultiConnect();
  }
  if (!connected && connect_mode_ == ConnectMode::kHiddenConnect) {
    connected = tryHiddenConnect();
  }
  if (!connected && connect_mode_ == ConnectMode::kCyclePower) {
    connected = tryCyclePower();
  }
  return connected ? ConnectMode::kConnected : connect_mode_;
}

bool Network::isConnected(wl_status_t* wifi_status) {
  // If wifi_status is passed, use that state, else check it now
  wl_status_t status;
  if (wifi_status == nullptr) {
    status = WiFi.status();
  } else {
    status = *wifi_status;
  }

  // If connected, set connect mode to fast connect for reconnect
  if (status == WL_CONNECTED) {
    connect_mode_ = ConnectMode::kFastConnect;
#ifdef MONITOR_MEMORY
    if (connect_start_ != std::chrono::steady_clock::time_point::min()) {
      Serial.printf("Network: Connected to %s, IP: %s\n", WiFi.SSID().c_str(),
                    WiFi.localIP().toString().c_str());
    }
#endif
    connect_start_ = std::chrono::steady_clock::time_point::min();
    return true;
  }
  return false;
}

bool Network::tryFastConnect() {
  wl_status_t wifi_status = WL_IDLE_STATUS;
  if (connect_start_ == std::chrono::steady_clock::time_point::min()) {
    // If first run of fast connect
    connect_start_ = std::chrono::steady_clock::now();
    if (strlen(WiFi.SSID().c_str())) {
      TRACEF("FastConnect: %s\n", WiFi.SSID().c_str());
      // Connect to previous saved WiFi, if one exists,
      wifi_status = WiFi.begin();
    } else {
      // else start WiFi search and exit fast connect mode
      connect_start_ = std::chrono::steady_clock::time_point::min();
      connect_mode_ = ConnectMode::kScanning;
      return false;
    }
  } else {
    // Subsequent attempt to fast connect, update connection status
    wifi_status = WiFi.status();
  }
  // Return connected, reset fast connect, but stay in fast connect mode
  if (isConnected(&wifi_status)) {
    return true;
  }
  // If connection to AP failed or timeout passed, start WiFi search
  std::chrono::steady_clock::duration connect_duration =
      std::chrono::steady_clock::now() - connect_start_;
  if (wifi_status == WL_CONNECT_FAILED || connect_duration > connect_timeout_) {
    TRACEF(
        "Failed connecting to %s after %lldms\n", WiFi.SSID().c_str(),
        std::chrono::duration_cast<std::chrono::milliseconds>(connect_duration)
            .count());
    connect_start_ = std::chrono::steady_clock::time_point::min();
    connect_mode_ = ConnectMode::kScanning;
  }
  // Stay in fast connect mode, but return not connected yet
  return false;
}

bool Network::tryScanning() {
  if (scan_start_ == std::chrono::steady_clock::time_point::min()) {
    TRACELN(F("Starting scan"));
    // If first run of WiFi scan
    scan_start_ = std::chrono::steady_clock::now();
    // Clean previous scan
    WiFi.scanDelete();
    // Remove previous WiFi SSID/password
    WiFi.disconnect();
    // Start wifi scan in async mode
    WiFi.scanNetworks(true);
  }
  int8_t scan_result = WiFi.scanComplete();
  std::chrono::steady_clock::duration scan_duration =
      std::chrono::steady_clock::now() - scan_start_;
  if (scan_result > 0) {
    TRACEF("Scan found %d APs after %lldms\n", scan_result,
           std::chrono::duration_cast<std::chrono::milliseconds>(scan_duration)
               .count());
    // If scan finished and found networks
    // Clear internal IDs of WiFi network
    for (auto& wifi_ap : wifi_aps_) {
      wifi_ap.id = -1;
      wifi_ap.failed_connecting = false;
    }
    // Set IDs for found and known WiFi networks
    // For each found AP, get its details and
    for (int8_t i = 0; i < scan_result; i++) {
      NetworkInfo network_info{.id = i};
      populateNetworkInfo(network_info);
      // ... check against known APs and
      for (auto& wifi_ap : wifi_aps_) {
        // ... if it matches one
        if (network_info.ssid == wifi_ap.ssid) {
          if (wifi_ap.id == -1) {
            // ... if it is the first AP for a given SSID
            TRACEF("Found AP %s, RSSI: %d, ID: %d\n", wifi_ap.ssid.c_str(),
                   WiFi.RSSI(i), i);
            wifi_ap.id = i;
          } else if (WiFi.RSSI(i) > WiFi.RSSI(wifi_ap.id)) {
            // ... set the ID of the one with the strongest signal
            TRACEF("Update AP %s, RSSI: %d, ID: %d\n", wifi_ap.ssid.c_str(),
                   WiFi.RSSI(i), i);
            wifi_ap.id = i;
          } else {
            TRACEF("Weaker AP %s ignored, RSSI: %d, ID: %d\n",
                   wifi_ap.ssid.c_str(), WiFi.RSSI(i), i);
          }
          break;
        }
      }
    }
    // Sort WiFi networks by RSSI and if they were found
    std::sort(wifi_aps_.begin(), wifi_aps_.end(), sortRssi);
    // Change to multi connect and set the first network to be tried
    connect_mode_ = ConnectMode::kMultiConnect;
    current_wifi_ap_ = wifi_aps_.begin();
    scan_start_ = std::chrono::steady_clock::time_point::min();
  } else if (scan_result == 0 || scan_duration > scan_timeout_) {
    // If the scan finished but didn't find any networks or it timed out,
    // try to connect to hidden networks
    TRACEF("No APs found after %lldms\n",
           std::chrono::duration_cast<std::chrono::milliseconds>(scan_duration)
               .count());
    connect_mode_ = ConnectMode::kHiddenConnect;
    current_wifi_ap_ = wifi_aps_.begin();
    scan_start_ = std::chrono::steady_clock::time_point::min();
  }
  // Scan never results in connection to AP
  return false;
}

bool Network::tryMultiConnect() {
  if (current_wifi_ap_ == wifi_aps_.end() || current_wifi_ap_->id == -1) {
    // On reaching the last or last known AP, try connecting to hidden APs
    connect_mode_ = ConnectMode::kHiddenConnect;
    // Try APs from begining
    current_wifi_ap_ = wifi_aps_.begin();
    WiFi.scanDelete();
    return false;
  }

  wl_status_t wifi_status = WL_IDLE_STATUS;
  if (connect_start_ == std::chrono::steady_clock::time_point::min()) {
    // If the first run of connecting to a network
    connect_start_ = std::chrono::steady_clock::now();
    // Get all the details of the network to connect to
    NetworkInfo network_info{.id = current_wifi_ap_->id};
    populateNetworkInfo(network_info);

    // If the SSID matches, try connecting, else skip it
    if (current_wifi_ap_->ssid == network_info.ssid) {
      TRACEF("Connecting to %s\n", network_info.ssid.c_str());
      wifi_status = WiFi.begin(network_info.ssid.c_str(),
                               current_wifi_ap_->password.c_str(),
                               network_info.channel, network_info.bssid);
    } else {
      current_wifi_ap_++;
      connect_start_ = std::chrono::steady_clock::time_point::min();
      return false;
    }
  } else {
    wifi_status = WiFi.status();
  }

  if (isConnected(&wifi_status)) {
    WiFi.scanDelete();
    return true;
  }
  std::chrono::steady_clock::duration connect_duration =
      std::chrono::steady_clock::now() - connect_start_;
  if (wifi_status == WL_CONNECT_FAILED || connect_duration > connect_timeout_) {
    // On connection failure, mark AP as failed, reset connection timer, try
    // next AP
    TRACEF("Failed connecting to %s after %lldms\n",
           current_wifi_ap_->ssid.c_str(),
           std::chrono::duration_cast<std::chrono::seconds>(connect_duration)
               .count());
    current_wifi_ap_->failed_connecting = true;
    connect_start_ = std::chrono::steady_clock::time_point::min();
    current_wifi_ap_++;
    return false;
  }
  // Stay in multi-connect mode, but return still trying to connect
  return false;
}

bool Network::tryHiddenConnect() {
  if (current_wifi_ap_ == wifi_aps_.end()) {
    // On reaching the last AP, cycle modem power before returning to scan mode
    connect_mode_ = ConnectMode::kCyclePower;
    return false;
  }

  wl_status_t wifi_status = WL_IDLE_STATUS;
  if (connect_start_ == std::chrono::steady_clock::time_point::min()) {
    // If the first run of connecting to a hidden network
    connect_start_ = std::chrono::steady_clock::now();
    while (current_wifi_ap_ != wifi_aps_.end()) {
      if (current_wifi_ap_->failed_connecting) {
        current_wifi_ap_++;
      } else {
        break;
      }
    }
    if (current_wifi_ap_ == wifi_aps_.end()) {
      return false;
    }
    TRACEF("Connecting to %s\n", current_wifi_ap_->ssid.c_str());
    // Try to connect to the next elligible AP
    wifi_status = WiFi.begin(current_wifi_ap_->ssid.c_str(),
                             current_wifi_ap_->password.c_str());
  } else {
    wifi_status = WiFi.status();
  }

  if (isConnected(&wifi_status)) {
    // Connected to AP, so set fast connect mode if connection drops
    return true;
  }
  std::chrono::steady_clock::duration connect_duration =
      std::chrono::steady_clock::now() - connect_start_;
  if (wifi_status == WL_CONNECT_FAILED || connect_duration > connect_timeout_) {
    // On failure, reset connection timer, try next AP
    TRACEF(
        "Failed connecting to %s after %lldms\n",
        current_wifi_ap_->ssid.c_str(),
        std::chrono::duration_cast<std::chrono::milliseconds>(connect_duration)
            .count());
    connect_start_ = std::chrono::steady_clock::time_point::min();
    current_wifi_ap_++;
    return false;
  }
  // Stay in hidden-connect mode, but return still trying to connect
  return false;
}

bool Network::tryCyclePower() {
  const auto mode = WiFi.getMode();
  // If WiFi modem is not powered off, turn it off
  TRACEF("Changing from WiFi mode: %d\n", mode);
  if (mode != WIFI_OFF) {
    WiFi.mode(WIFI_OFF);
  } else {
// In the next cycle, turn it back on and try to fast connect
#ifdef ESP32
    WiFi.mode(WIFI_MODE_STA);
#else
    WiFi.mode(WIFI_STA);
#endif
    connect_mode_ = ConnectMode::kFastConnect;
  }
  // Power cycle never results in connection to AP
  return false;
}

int Network::setClock(std::chrono::seconds timeout_s) {
  // #TODO: Make operation non-blocking
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  TRACELN(F("Network: Waiting for NTP time sync"));
  Serial.print(F("\t"));
  time_t nowSecs = time(nullptr);
  std::chrono::milliseconds delay_duration(500);
  unsigned int tries = 0;
  while (nowSecs < 8 * 3600 * 2) {
    delay(delay_duration.count());
    tries++;
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
    if (tries * delay_duration > timeout_s) {
      Serial.println();
      Serial.print(F("Timed out after "));
      Serial.print(static_cast<long>(timeout_s.count()));
      Serial.println("s");
      return 1;
    }
  }
  Serial.println();

  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
  return 0;
}

bool Network::populateNetworkInfo(NetworkInfo& network_info) {
  if (network_info.id < 0 || network_info.id > 255) {
    return false;
  }
  uint8_t id = static_cast<uint8_t>(network_info.id);
#ifndef ESP32
  return WiFi.getNetworkInfo(id, network_info.ssid, network_info.encType,
                             network_info.rssi, network_info.bssid,
                             network_info.channel, network_info.hidden);
#else
  return WiFi.getNetworkInfo(id, network_info.ssid, network_info.encType,
                             network_info.rssi, network_info.bssid,
                             network_info.channel);
#endif
}

bool Network::sortRssi(const WiFiAP& lhs, const WiFiAP& rhs) {
  if (lhs.id == -1) {
    return false;
  }
  if (rhs.id == -1) {
    return true;
  }
  return WiFi.RSSI(lhs.id) > WiFi.RSSI(rhs.id);
}

}  // namespace inamata
