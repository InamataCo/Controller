
#ifndef ESP_MONITOR_MQTT
#define ESP_MONITOR_MQTT

#include <PubSubClient.h>
#include <WiFi.h>
#include <sstream>

namespace bernd_box {

class Mqtt {
 public:
  /**
   * Class to simplify MQTT sending and receiving tasks
   *
   * \param wifi_client An ESP WiFi client to perform TCP/IP and UDP operations
   * \param client_id Name of the node
   * \param mqtt_server IP address of the MQTT broker
   */
  Mqtt(WiFiClient& wifi_client, const char* client_id, const char* mqtt_server)
      : client_(wifi_client),
        client_id_(client_id),
        error_topic("tele/" + std::string(client_id_) + "/error"),
        server_ip_address_(mqtt_server),
        server_port_(1883) {
    client_.setServer(server_ip_address_, server_port_);
  }

  /**
   * Loop until connected to MQTT server or tries exceeded
   *
   * \param max_attempts The maximum connection attempts to be made
   * \return False if the connection attempts were exceeded
   */
  bool connect(uint max_attempts = 1) {
    Serial.printf("MQTT: Attempting to connect to %s\n", server_ip_address_);

    for (uint connect_attempts = 0;
         !client_.connected() && connect_attempts < max_attempts;
         connect_attempts++) {
      Serial.printf("MQTT connecting to %s. Try #%d\n", server_ip_address_,
                    connect_attempts + 1);

      client_.connect(client_id_.c_str());
    }

    return client_.connected();
  }

  /**
   * Checks whether the ESP is connected to an MQTT broker
   *
   * \return isConnected True if connected
   */
  bool isConnected() { return client_.connected(); }

  /**
   * Gets the IP address and port as a string in the format "192.168.0.1:1883"
   *
   * \return String with IP address and port
   */
  String getBrokerAddress() {
    return String(server_ip_address_) + ":" + String(server_port_);
  }

  /**
   * Non-blocking receive call
   *
   * Handles the incoming messages and keeps the MQTT connection alive
   */
  void receive() { client_.loop(); }

  /**
   * Send a double value on a topic
   *
   * \param name Suffix of the topic to publish on
   * \param value Double value to send
   */
  void send(const std::string& name, double value) {
    // Success if lenght is non-negative and shorter than buffer
    char value_buffer[20];
    int length = snprintf(value_buffer, sizeof(value_buffer), "%f", value);

    if (length >= 0 && length < sizeof(value_buffer)) {
      send(name, value_buffer);
    } else {
      std::string error = "Failed to convert double to string. Name = ";
      error += name;
      error += ", length = ";
      error += length;
      sendError("mqtt::send", error);
    }
  }

  /**
   * Send an integer value on a topic
   *
   * \param name Suffix of the topic to publish on
   * \param value Integer value to send
   */
  void send(const std::string& name, int value) {
    // Success if lenght is non-negative and shorter than buffer
    char value_buffer[20];
    int length = snprintf(value_buffer, sizeof(value_buffer), "%d", value);

    if (length >= 0 && length < sizeof(value_buffer)) {
      send(name, value_buffer);
    } else {
      std::string error = "Failed to convert int to string. Name = ";
      error += name;
      error += ", length = ";
      error += length;
      sendError("mqtt::send", error);
    }
  }

  /**
   * Send a boolean value on a topic
   *
   * \param name Suffix of the topic to publish on
   * \param value Boolean value to send
   */
  void send(const std::string& name, bool value) {
    if (value == true) {
      send(name, "true");
    } else {
      send(name, "false");
    }
  }

  /**
   * Send a C-String on a topic
   *
   * The string has to be null terminated
   *
   * \param name Suffix of the topic to publish on
   * \param value String to send
   */
  void send(const std::string& name, const char* value) {
    std::string topic = "tele/" + client_id_ + "/" + name;
    client_.publish(topic.c_str(), value);
  }

  /**
   * Sends a message to the error topic
   *
   * \param who From which function the error originates
   * \param message String stating the error
   */
  void sendError(const std::string& who, const std::string& message,
                 bool additional_serial_log = true) {
    std::string error = who;
    error += ": ";
    error += message;
    client_.publish(error_topic.c_str(), error.c_str());

    if (additional_serial_log == true) {
      Serial.println(error.c_str());
    }
  }

  void sendError(const String& who, const String& message,
                 bool additional_serial_log) {
    String error = who;
    error += ": ";
    error += message;
    client_.publish(error_topic.c_str(), error.c_str());

    if (additional_serial_log == true) {
      Serial.println(error.c_str());
    }
  }

 private:
  PubSubClient client_;
  std::string client_id_;
  std::string error_topic;
  const char* server_ip_address_;
  const uint server_port_;
};

}  // namespace bernd_box

#endif