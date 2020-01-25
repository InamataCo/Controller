
#ifndef ESP_MONITOR_MQTT
#define ESP_MONITOR_MQTT

#include <PubSubClient.h>
#include <WiFi.h>

#include <functional>
#include <map>
#include <vector>

#include "ArduinoJson.h"
#include "config.h"
#include "utils/setupNode.h"

namespace bernd_box {

using namespace std::placeholders;
using callback_map =
    std::map<String, std::function<void(char*, uint8_t*, unsigned int)>>;

class Mqtt {
 public:
  /**
   * Class to simplify MQTT sending and receiving tasks
   *
   * \param wifi_client An ESP WiFi client to perform TCP/IP and UDP operations
   */
  Mqtt(WiFiClient& wifi_client);

  /**
   * Loop until connected to MQTT server or tries exceeded
   *
   * \param max_attempts The maximum connection attempts to be made
   * \return 0 on success
   */
  int connect(uint max_attempts = 3);

  /**
   * Checks whether the ESP is connected to an MQTT broker
   *
   * \return isConnected True if connected
   */
  bool isConnected();

  /**
   * Updates and connects to a new MQTT server (broker)
   *
   * \param server_ip_address_ The IP address of the new broker
   * \param client_id ID of the controller
   * \return 0 on success
   */
  int switchBroker(const String& server_ip_address_,
                   const String& client_id = String());

  /**
   * Gets the IP address and port as a string in the format "192.168.0.1:1883"
   *
   * \return String with IP address and port
   */
  String getBrokerAddress();

  /**
   * Non-blocking receive call
   *
   * Handles the incoming messages and keeps the MQTT connection alive
   */
  void receive();

  /**
   * Send a double value on a topic
   *
   * \param name Suffix of the topic to publish on
   * \param value Double value to send
   */
  void send(const String& name, double value);

  /**
   * Send an integer value on a topic
   *
   * \param name Suffix of the topic to publish on
   * \param value Integer value to send
   */
  void send(const String& name, int value);

  /**
   * Send a boolean value on a topic
   *
   * \param name Suffix of the topic to publish on
   * \param value Boolean value to send
   */
  void send(const String& name, bool value);

  /**
   * Send a JSON document
   *
   * \param name Suffix of the topic to publish on
   * \param value JSON document to send
   */
  void send(const String& name, const DynamicJsonDocument& doc);

  /**
   * Send a C-String on a topic
   *
   * The string has to be null terminated
   *
   * \param name Suffix of the topic to publish on
   * \param value String to send
   * \param length Optional, length of the payload
   */
  void send(const String& name, const char* value, size_t length = 0);

  void sendRegister();

  /**
   * Sends a message to the error topic
   *
   * \param who From which function the error originates
   * \param message String stating the error
   */
  void sendError(const String& who, const String& message,
                 const bool additional_serial_log = true);
  /**
   * Add an action that can be performed
   *
   * Actions are commands formatted as JSON objects and can contain arbitrary
   * parameters
   *
   * \param name The name of the action to subscribe to
   * \param callback The function to call when a message for the action arrives
   */
  void addAction(const String& name,
                 std::function<void(char*, uint8_t*, unsigned int)> callback);

  /**
   * Removes an action
   *
   * If it can not be found, the callbacks_ map is in the desired state
   *
   * \param name Name of the subscribed action
   */
  void removeAction(const String& topic);

  /**
   * Returns a const reference to the MQTT callback map
   *
   * \return The callback map
   */
  const callback_map& getCallbackMap();

 private:
  /**
   * Calls the function registered to an MQTT topic
   *
   * \param topic The topic the message arrived on
   * \param payload The message payload
   * \param length The length of the message payload
   */
  void handleCallback(char* topic, uint8_t* payload, unsigned int length);

  PubSubClient client_;
  String client_id_;
  String error_topic;
  String server_ip_address_;
  const uint server_port_;
  callback_map callbacks_;
  uint8_t default_qos_;
};

}  // namespace bernd_box

#endif
