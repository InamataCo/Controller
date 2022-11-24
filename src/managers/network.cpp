#include "network.h"

namespace inamata {

Network::Network(std::vector<WiFiAP>& wifi_aps, String& controller_name)
    : wifi_aps_(std::move(wifi_aps)),
      controller_name_(std::move(controller_name)) {
  Serial.println(F("Network: Searching for the following networks:"));
  for (const WiFiAP& wifi_ap : wifi_aps_) {
    Serial.printf("\t%s\n", wifi_ap.ssid.c_str());
  }
}

bool Network::connect() {
  // If already connected, return connected status
  wl_status_t wifi_status = WiFi.status();
  if (wifi_status == WL_CONNECTED) {
    connect_mode_ = ConnectMode::kFastConnect;
    connect_start_ = std::chrono::steady_clock::time_point::min();
    return true;
  }

  if (connect_mode_ == ConnectMode::kFastConnect) {
    // If first run of fast connect
    if (connect_start_ == std::chrono::steady_clock::time_point::min()) {
      connect_start_ = std::chrono::steady_clock::now();
      // Connect to previous saved WiFi, if one exists,
      if (strlen(WiFi.SSID().c_str())) {
        WiFi.begin();
        wifi_status = WiFi.status();
      } else {
        // else start WiFi search and exit fast connect mode
        connect_start_ = std::chrono::steady_clock::time_point::min();
        connect_mode_ = ConnectMode::kScanning;
      }
    }
    // Return connected, reset fast connect, but stay in fast connect mode
    if (wifi_status == WL_CONNECTED) {
      connect_start_ = std::chrono::steady_clock::time_point::min();
      return true;
    }
    // If connection to AP failed or timeout passed, start WiFi search
    if (wifi_status == WL_CONNECT_FAILED ||
        std::chrono::steady_clock::now() - connect_start_ > connect_timeout_) {
      connect_start_ = std::chrono::steady_clock::time_point::min();
      connect_mode_ = ConnectMode::kScanning;
    } else {
      // Stay in fast connect mode, but return not connected yet
      return false;
    }
  }

  // In multi-AP WiFi scanning state
  if (connect_mode_ == ConnectMode::kScanning) {
    // If first run of WiFi scan
    if (scan_start_ == std::chrono::steady_clock::time_point::min()) {
      scan_start_ = std::chrono::steady_clock::now();
      // Clean previous scan
      WiFi.scanDelete();
      // Remove previous WiFi SSID/password
      WiFi.disconnect();
      // Start wifi scan in async mode
      WiFi.scanNetworks(true);
    }
    int8_t scan_result = WiFi.scanComplete();
    if (scan_result > 0) {
      // If scan finished and found networks
      // Clear internal IDs of WiFi network
      for (auto& wifi_ap : wifi_aps_) {
        wifi_ap.id = -1;
        wifi_ap.failed_connecting = false;
      }
      // Set IDs for found and known WiFi networks
      for (int8_t i = 0; i < scan_result; i++) {
        NetworkInfo network_info{.id = i};
        populateNetworkInfo(network_info);
        for (auto& wifi_ap : wifi_aps_) {
          if (network_info.ssid == wifi_ap.ssid) {
            wifi_ap.id = i;
            continue;
          }
        }
      }
      // Sort WiFi networks by RSSI and if they were found
      std::sort(wifi_aps_.begin(), wifi_aps_.end(), sortRssi);
      // Change to multi connect and set the first network to be tried
      connect_mode_ = ConnectMode::kMultiConnect;
      current_wifi_ap_ = wifi_aps_.begin();
      scan_start_ = std::chrono::steady_clock::time_point::min();
    } else if (scan_result == 0 ||
               std::chrono::steady_clock::now() - scan_start_ > scan_timeout_) {
      // If the scan finished but didn't find any networks or it timed out,
      // try to connect to hidden networks
      connect_mode_ = ConnectMode::kHiddenConnect;
      current_wifi_ap_ = wifi_aps_.begin();
      scan_start_ = std::chrono::steady_clock::time_point::min();
    }
  }

  if (connect_mode_ == ConnectMode::kMultiConnect) {
    if (current_wifi_ap_ == wifi_aps_.end() || current_wifi_ap_->id == -1) {
      // On reaching the last or last known AP, try connecting to hidden APs
      connect_mode_ = ConnectMode::kHiddenConnect;
      // Try APs from begining
      current_wifi_ap_ = wifi_aps_.begin();
      WiFi.scanDelete();
      return false;
    }

    if (connect_start_ == std::chrono::steady_clock::time_point::min()) {
      // If the first run of connecting to a network
      connect_start_ = std::chrono::steady_clock::now();
      // Get all the details of the network to connect to
      NetworkInfo network_info{.id = current_wifi_ap_->id};
      populateNetworkInfo(network_info);

      // If the SSID matches, try connecting, else skip it
      if (current_wifi_ap_->ssid == network_info.ssid) {
        WiFi.begin(network_info.ssid.c_str(),
                   current_wifi_ap_->password.c_str(), network_info.channel,
                   network_info.bssid);
      } else {
        current_wifi_ap_++;
        connect_start_ = std::chrono::steady_clock::time_point::min();
        return false;
      }
    }

    wl_status_t wifi_status = WiFi.status();
    if (wifi_status == WL_CONNECTED) {
      connect_mode_ = ConnectMode::kFastConnect;
      connect_start_ = std::chrono::steady_clock::time_point::min();
      WiFi.scanDelete();
      return true;
    }
    if (wifi_status == WL_CONNECT_FAILED ||
        std::chrono::steady_clock::now() - connect_start_ > connect_timeout_) {
      // On connection failure, mark AP as failed, reset connection timer, try
      // next AP
      current_wifi_ap_->failed_connecting = true;
      connect_start_ = std::chrono::steady_clock::time_point::min();
      current_wifi_ap_++;
      return false;
    }
  }

  if (connect_mode_ == ConnectMode::kHiddenConnect) {
    if (current_wifi_ap_ == wifi_aps_.end()) {
      // On reaching the last AP, return to scan mode
      connect_mode_ = ConnectMode::kScanning;
      return false;
    }

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

      wl_status_t wifi_status = WiFi.begin(current_wifi_ap_->ssid.c_str(),
                                           current_wifi_ap_->password.c_str());
      if (wifi_status == WL_CONNECTED) {
        // Connected to AP, so set fast connect mode if connection drops
        connect_mode_ = ConnectMode::kFastConnect;
        connect_start_ = std::chrono::steady_clock::time_point::min();
        return true;
      }
      if (wifi_status == WL_CONNECT_FAILED ||
          std::chrono::steady_clock::now() - connect_start_ >
              connect_timeout_) {
        // On failure, reset connection timer, try next AP
        connect_start_ = std::chrono::steady_clock::time_point::min();
        current_wifi_ap_++;
        return false;
      }
    }
  }

  return false;
}

void Network::printState() {
  Serial.printf("Connected: %d\n", connect());
  Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
}

int Network::setClock(std::chrono::seconds timeout_s) {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.println(F("Network::setClock: Waiting for NTP time sync"));
  Serial.print("\t");
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

bool Network::isTimeSet() { return false; }

String Network::getSsid() { return WiFi.SSID(); }

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
