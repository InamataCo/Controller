#include "network.h"

namespace bernd_box {

Network::Network(const char* ssid, const char* password)
    : ssid_(ssid),
      password_(password),
      connect_wait_duration_(std::chrono::milliseconds(500)) {
  wiFiMulti_.addAP(ssid_, password_);
}

bool Network::connect(std::chrono::duration<int> timeout) {
  Serial.print(F("Network::connect: Connecting to "));
  Serial.println(ssid_);

  while (wiFiMulti_.run() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println();

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

String Network::pingSdgServer() {
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

String Network::getCoordinatorLocalIpAddress() {
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
