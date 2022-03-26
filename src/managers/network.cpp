#include "network.h"

namespace bernd_box {

Network::Network(std::vector<WiFiAP>& wifi_aps)
    : wifi_aps_(std::move(wifi_aps)) {
  for (const WiFiAP& wifi_ap : wifi_aps_) {
    wiFiMulti_.addAP(wifi_ap.ssid.c_str(), wifi_ap.password.c_str());
  }
}

bool Network::connect(std::chrono::steady_clock::duration timeout) {
  Serial.println(F("Network::connect: Searching for the following networks:"));
  for (const WiFiAP& wifi_ap : wifi_aps_) {
    Serial.printf("\t%s\n", wifi_ap.ssid.c_str());
  }

  const auto wifi_connect_start = std::chrono::steady_clock::now();
  while (wiFiMulti_.run() != WL_CONNECTED) {
    if (std::chrono::steady_clock::now() - wifi_connect_start > timeout) {
      return false;
    }
  }

  Serial.print("\tConnected! IP address is ");
  Serial.println(WiFi.localIP());
  return true;
}

void Network::printState() {
  Serial.printf("Connected: %d\n", isConnected());
  Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
}

int Network::setClock(std::chrono::seconds timeout_s) {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.println(F("Network::setClock: Waiting for NTP time sync"));
  Serial.print("\t");
  time_t nowSecs = time(nullptr);
  std::chrono::milliseconds delay_duration(500);
  uint tries = 0;
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

bool Network::isConnected() { return WiFi.status() == WL_CONNECTED; }

}  // namespace bernd_box
