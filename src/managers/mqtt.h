#pragma once

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include <functional>
#include <map>
#include <vector>

#include "managers/server.h"
#include "utils/setupNode.h"

namespace bernd_box {

using namespace std::placeholders;

/**
 * MQTT related functionality
 *
 * Connects to MQTT broker, send MQTT messages and handles incoming ones by
 * delegating them the the appropriate controller (peripheral / task)
 */
class Mqtt {
 public:
  /**
   * Class to simplify MQTT sending and receiving tasks
   *
   * \param wifi_client An ESP WiFi client to perform TCP/IP and UDP operations
   */
  Mqtt(WiFiClient& wifi_client,
       std::function<std::vector<String>()> get_factory_names,
       Server::Callback library_callback, Server::Callback task_callback);

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
   * Subscribe to the action and object topics
   *
   * @return int 0 on success
   */
  int subscribe();

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

 private:
  /**
   * Calls the function registered to an MQTT topic
   *
   * \param topic The topic the message arrived on
   * \param payload The message payload
   * \param length The length of the message payload
   */
  void handleCallback(char* topic, uint8_t* payload, unsigned int length);

  const String getMacString();

  PubSubClient client_;
  String client_id_;
  String error_topic;
  String server_ip_address_;
  const uint server_port_ = 1883;

  /// Object prefix for MQTT messages. Includes trailing slash delimiter
  const __FlashStringHelper* object_prefix_ = F("object/");
  /// Function to handle object messages
  Server::Callback peripheral_callback_;

  /// Task prefix for MQTT messages. Includes trailing slash delimiter
  const __FlashStringHelper* task_prefix_ = F("task/");
  /// Function to handle task messages
  Server::Callback task_callback_;

  std::function<std::vector<String>()> get_factory_names_;

  uint8_t default_qos_ = 1;
};  // namespace bernd_box

}  // namespace bernd_box
