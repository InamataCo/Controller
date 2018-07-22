
#ifndef ESP_MONITOR_MQTT
#define ESP_MONITOR_MQTT

#include <PubSubClient.h>
#include <WiFi.h>
#include <sstream>

namespace bernd_box {

class Mqtt {
 public:
  Mqtt(WiFiClient& wifiClient, const char* client_id, const char* mqtt_server)
      : client_(wifiClient),
        client_id_(client_id),
        server_ip_address_(mqtt_server),
        server_port_(1883) {}

  /**
   * Loop until connected to MQTT server or tries exceeded
   */
  bool connect(uint max_attempts = 1) {
    client_.setServer(server_ip_address_, server_port_);

    for (uint connect_attempts = 0;
         !client_.connected() && connect_attempts < max_attempts;
         connect_attempts++) {
      Serial.printf("MQTT connecting to %s. Try #%d\n", server_ip_address_,
                    connect_attempts + 1);

      // connect now
      if (client_.connect(client_id_.c_str())) {
        Serial.println("connected");
      } else {
        Serial.printf("MQTT connection Failed (%d)\nRetrying in 5 seconds\n",
                      client_.state());
        delay(5000);
      }
    }

    return client_.connected();
  }

  void send(const std::string& name, float value) {
    // Success if lenght is non-negative and shorter than buffer
    char value_buffer[20];
    int length = snprintf(value_buffer, sizeof(value_buffer), "%lf", value);

    if (length >= 0 && length < sizeof(value_buffer)) {
      std::string value_text = value_buffer;
      std::string topic = "tele/" + client_id_ + "/" + name;
      client_.publish(topic.c_str(), value_text.c_str());
    } else {
      Serial.printf("Failed to convert float to string. Length = %d\n", length);
    }
  }

 private:
  PubSubClient client_;
  std::string client_id_;
  const char* server_ip_address_;
  const uint server_port_;
};

}  // namespace bernd_box

#endif