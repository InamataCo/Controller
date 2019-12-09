
#ifndef ESP_MONITOR_MQTT
#define ESP_MONITOR_MQTT

#include <PubSubClient.h>
#include <WiFi.h>

namespace bernd_box {

using namespace std::placeholders;

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
        error_topic("tele/" + String(client_id_) + "/error"),
        server_ip_address_(mqtt_server),
        server_port_(1883) {
    uint8_t mac_address[6];
    esp_efuse_mac_get_default(mac_address);

    client_.setServer(server_ip_address_, server_port_);
    // client_.setCallback(std::bind(&Mqtt::handleCallback, this, _1, _2, _3));
    // client_.subscribe(String("sdg/action/" + String(client_id_)).c_str());
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
  void send(const String& name, double value) {
    // Success if lenght is non-negative and shorter than buffer
    char value_buffer[20];
    int length = snprintf(value_buffer, sizeof(value_buffer), "%f", value);

    if (length >= 0 && length < sizeof(value_buffer)) {
      send(name, value_buffer);
    } else {
      String error = "Failed to convert double to string. Name = ";
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
  void send(const String& name, int value) {
    // Success if lenght is non-negative and shorter than buffer
    char value_buffer[20];
    int length = snprintf(value_buffer, sizeof(value_buffer), "%d", value);

    if (length >= 0 && length < sizeof(value_buffer)) {
      send(name, value_buffer);
    } else {
      String error = "Failed to convert int to string. Name = ";
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
  void send(const String& name, bool value) {
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
  void send(const String& name, const char* value) {
    String topic = "tele/" + client_id_ + "/" + name;
    client_.publish(topic.c_str(), value);
  }

  /**
   * Sends a message to the error topic
   *
   * \param who From which function the error originates
   * \param message String stating the error
   */
  void sendError(const String& who, const String& message,
                 bool additional_serial_log = true) {
    String error = who;
    error += ": ";
    error += message;
    client_.publish(error_topic.c_str(), error.c_str());

    if (additional_serial_log == true) {
      Serial.println(error.c_str());
    }
  }

  /**
   * Add an action that can be performed
   * 
   * Actions are commands formatted as JSON objects and can contain arbitrary parameters
   * 
   * \param name The name of the action to subscribe to
   * \param callback The function to call when a message for the action arrives
   * \return True if adding the action succeeded
   */
  bool addAction(const String& name, std::function<void(char*, uint8_t*, unsigned int)> callback) {
    bool error = false;

    // Add callback to map, to be called when a message for the action arrives
    auto result = callbacks_.emplace(std::make_pair(name, callback));
    if(!result.second) {
      callbacks_.erase(result.first);
      result = callbacks_.emplace(std::make_pair(name, callback));
      if(!result.second) {
        sendError("mqtt::addAction", "Unable to register callback: " + name);
        error = true;
      }
    }

    // If the callback could be registered, subscribe to the topic
    if(!error) {
      error = !client_.subscribe(name.c_str());
      if(error) {
        sendError("mqtt::addAction", "Unable to subscribe to action: " + name);
      }
    }

    return error;
  }

  /**
   * Unsubscribes to an MQTT topic
   * 
   * \param topic The MQTT topic to unsubscribe from
   * \return True if unsubscribing succeeded
   */
  bool unsubscribe(const String& topic) {
    bool error = !client_.unsubscribe(topic.c_str());

    // Remove the callback. If it can not be found, the callbacks_ map is in the desired state
    if(!error) {
      callbacks_.erase(topic.c_str());
    }

    return error;
  }

 private:
  /**
   * Calls the function registered to an MQTT topic
   * 
   * 
   */
  void handleCallback(char* topic, uint8_t* message, unsigned int length) {
    auto it = callbacks_.find(topic);

    if(it != callbacks_.end()) {
      it->second(topic, message, length);
    } else {
      sendError("mqtt::handleCallback", "No registered callback to subscribed topic");
    }
  }

  PubSubClient client_;
  String client_id_;
  String error_topic;
  const char* server_ip_address_;
  const uint server_port_;
  std::map<String, std::function<void(char*, uint8_t*, unsigned int)>> callbacks_;
};

}  // namespace bernd_box

#endif
