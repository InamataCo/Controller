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

#include <HTTPClient.h>

#include <chrono>

#include "ArduinoJson.h"
#include "config.h"
#include "utils/setupNode.h"

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
   * Connects to the configured WiFi access point and contacts the SDG server
   *
   * Blocks until the connection has been made. Also sends a JSON to the SDG
   * server containing its UUID and MAC address.
   *
   * \param timeout The length of time to wait until aborting
   * \return True if successful
   */
  bool connect(std::chrono::duration<int> timeout) {
    Serial.print(F("Network::connect: Connecting to "));
    Serial.println(ssid_);

    WiFi.begin(ssid_, password_);

    std::chrono::milliseconds total_wait_time(0);

    // Attempt to connect until timeout is reached
    Serial.print("\t");
    while (WiFi.status() != WL_CONNECTED && total_wait_time < timeout) {
      delay(std::chrono::duration_cast<std::chrono::milliseconds>(
                connect_wait_duration_)
                .count());
      total_wait_time += connect_wait_duration_;
      Serial.print(".");
    }
    Serial.println();

    bool connected = isConnected();
    if (connected) {
      Serial.print("\tConnected! IP address is ");
      Serial.println(WiFi.localIP());
    }

    return connected;
  }

  String pingSdgServer() {
    // Advertise node to SDG server: uuid + mac_address
    DynamicJsonDocument ping_json(JSON_OBJECT_SIZE(2) + 77);

    // Add the UUID entry
    std::vector<uint8_t> uuid = getUuid();
    ping_json["uuid"] = ESPRandom::uuidToString(uuid);

    // Add the MAC address entry
    ping_json["wifi_mac_address"] = WiFi.macAddress();

    // Calculate the size of the resultant serialized JSON, create a buffer of
    // that size and serialize the JSON into that buffer.
    // Add extra byte for the null terminator
    std::vector<char> ping_buf = std::vector<char>(measureJson(ping_json) + 1);
    serializeJson(ping_json, ping_buf.data(), ping_buf.size());

    // Connect to the SDG server with the certificate of the root CA
    httpClient_.begin(ping_url_, root_ca_);
    httpClient_.addHeader(F("Content-Type"), F("application/json"));

    // Tell the server our UUID and MAC address. With the request our external
    // IP address can be deduced.
    Serial.println(F("Network::pingPost: Sending ping to SDG server."));
    Serial.print("\t");
    Serial.println(ping_url_);
    Serial.print("\t");
    Serial.println(ping_buf.data());
    int httpCode = httpClient_.POST(ping_buf.data());

    String response = "";
    if (httpCode > 0) {
      response = httpClient_.getString();
    } else {
      Serial.print(F("Error on sending POST: "));
      Serial.println(httpCode);
    }

    httpClient_.end();
    return response;
  }

  /**
   * Asks the SDG server for the IP address of the local coordinator.
   *
   * The coordinator hosts the MQTT broker, which is required for local
   * communication.
   *
   * \return IP address of the local MQTT server, empty if not found
   */
  String getCoordinatorLocalIpAddress() {
    // Connect to the SDG server with the certificate of the root CA
    httpClient_.begin(ping_url_, root_ca_);
    httpClient_.addHeader(F("Content-Type"), F("application/json"));

    // Send a GET request to the server to get the coordinator's IP address
    Serial.println(F("Requesting local coordinator's IP address:"));
    Serial.print(F("\t"));
    Serial.println(ping_url_);
    int httpCode = httpClient_.GET();

    // Valid requests will be return a 2XX code
    if (httpCode < 200 || httpCode >= 300) {
      Serial.print(F("\tError on sending GET: "));
      Serial.println(httpCode);
      return "";
    }

    // Expect JSON response {"coordinator_local_ip_address":"XXX.XXX.XXX.XXX"}
    String response = httpClient_.getString();
    DynamicJsonDocument json(BB_MQTT_JSON_PAYLOAD_SIZE);
    DeserializationError error = deserializeJson(json, response);

    if (error) {
      Serial.print(F("\tdeserializeJson() failed: "));
      Serial.println(error.c_str());
      return "";
    }

    // Store the address to be returned. Empty on error.
    String coordinator_local_ip_address =
        json["coordinator_local_ip_address"].as<String>();
    if (coordinator_local_ip_address.isEmpty()) {
      Serial.println(
          F("\tNo local coordinator found. Check external IP address."));
    }

    return coordinator_local_ip_address;
  }

  /**
   * Prints the current WiFi state to the serial terminal
   */
  void printState() {
    Serial.printf("Connected: %d\n", isConnected());
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
  }

  // Not sure if WiFiClientSecure checks the validity date of the certificate.
  // Setting clock just to be sure...
  int setClock(std::chrono::seconds timeout_s) {
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
      if(tries * delay_duration > timeout_s) {
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

#endif
