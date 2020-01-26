#include "mqtt.h"

#include "library/library.h"

namespace bernd_box {

Mqtt::Mqtt(WiFiClient& wifi_client)
    : client_(wifi_client),
      server_port_(BB_MQTT_PORT),
      action_prefix_(F("action/")),
      object_prefix_(F("object/")),
      default_qos_(BB_DEFAULT_QOS) {
  uint8_t mac_address[6];
  esp_efuse_mac_get_default(mac_address);

  client_.setCallback(std::bind(&Mqtt::handleCallback, this, _1, _2, _3));
}

int Mqtt::connect(const uint max_attempts) {
  Serial.println(F("Mqtt::connect: Connecting to MQTT broker"));
  Serial.print(F("\t"));
  Serial.println(server_ip_address_.c_str());
  Serial.print(F("\t"));
  Serial.println(client_id_.c_str());

  // Empty client ID means that the UUID has not be set
  if (client_id_.isEmpty()) {
    Serial.println(F("\tNo client ID set. Aborting."));
    return 1;
  }

  // After x tries, it means there is a problem with the coordinator's IP
  // address or the broker service is not setup correctly
  bool connected = false;
  for (uint connect_attempts = 0; !connected && connect_attempts < max_attempts;
       connect_attempts++) {
    Serial.print(F("\t"));
    Serial.print(F("Connection try "));
    Serial.println(connect_attempts + 1);

    connected = client_.connect(client_id_.c_str());
  }

  bool connect_error = !client_.connected();
  if (!connect_error) {
    Serial.println(F("\tConnected!"));
    connect_error = !client_.subscribe(
        (String(F("action/")) + client_id_ + F("/+")).c_str(), 1);
    connect_error |= !client_.subscribe(
        (String(F("object/")) + client_id_ + F("/+")).c_str(), 1);
  } else {
    Serial.print(F("\tFailed to connect after "));
    Serial.print(max_attempts);
    Serial.println(F(" tries."));
    return connect_error;
  }

  int subscribe_error = subscribe();
  if (subscribe_error) {
    Serial.println(F("\tFailed to subscribe"));
    return 1;
  }

  // 0 on success, 1 on error
  return 0;
}

bool Mqtt::isConnected() { return client_.connected(); }

int Mqtt::subscribe() {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);
  const __FlashStringHelper* error_prefix = F("Failed to subscribe to: ");
  const __FlashStringHelper* success_prefix = F("Subscribed to: ");

  const String action_topic = String(action_prefix_) + client_id_ + F("/+");
  const bool action_success = client_.subscribe(action_topic.c_str(), 1);
  if (action_success) {
    Serial.print("\t");
    Serial.print(success_prefix);
    Serial.println(action_topic);
  } else {
    sendError(who, String(error_prefix) + action_topic);
    return 1;
  }

  const String object_topic = String(object_prefix_) + client_id_ + F("/+");
  const bool object_success = client_.subscribe(object_topic.c_str(), 1);
  if (object_success) {
    Serial.print("\t");
    Serial.print(success_prefix);
    Serial.println(object_topic);
  } else {
    sendError(who, String(error_prefix) + object_topic);
    return 1;
  }

  return 0;
}

int Mqtt::switchBroker(const String& server_ip_address,
                       const String& client_id) {
  int error = 0;
  // If the client ID changes, also change the ID on which errors are reported
  if (!client_id.isEmpty()) {
    client_id_ = client_id;
    error_topic = "tele/" + String(client_id_) + "/error";
  }

  // Simple sanity check for IP address value
  if (!server_ip_address.isEmpty()) {
    server_ip_address_ = server_ip_address;
  } else {
    error = 1;
  }

  // Disconnect and reconnect with the new client ID
  if (!error) {
    client_.disconnect();
    client_.setServer(server_ip_address_.c_str(), server_port_);
    error = connect();
  }
  return error;
}

String Mqtt::getBrokerAddress() {
  return server_ip_address_ + ":" + String(server_port_);
}

void Mqtt::receive() { client_.loop(); }

void Mqtt::send(const String& name, double value) {
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

void Mqtt::send(const String& name, int value) {
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

void Mqtt::send(const String& name, bool value) {
  if (value == true) {
    send(name, "true");
  } else {
    send(name, "false");
  }
}

void Mqtt::send(const String& name, const DynamicJsonDocument& doc) {
  // Calculate the size of the resultant serialized JSON, create a buffer of
  // that size and serialize the JSON into that buffer.
  // Add extra byte for the null terminator
  std::vector<char> buf = std::vector<char>(measureJson(doc) + 1);
  size_t n = serializeJson(doc, buf.data(), buf.size());
  if (n != buf.size() - 1) {
    sendError(String(F("Mqtt::send")),
              String(F("Only serialized ")) + String(n) + String(F(" of ")) +
                  String(buf.size()) + String(F(" bytes.")));
    Serial.println("Error while serializing JSON");
  }
  send(name, buf.data(), buf.size());
}

void Mqtt::send(const String& name, const char* value, size_t length) {
  String topic = "tele/" + client_id_ + "/" + name;
  bool error;
  if (length) {
    error = !client_.publish(topic.c_str(), value, length);
  } else {
    error = !client_.publish(topic.c_str(), value);
  }
  if (error) {
    sendError(String(F("Mqtt::send")),
              String(F("Failed sending tele MQTT message for ")) + name);
  }
}

void Mqtt::sendRegister() {
  Serial.println(
      F("Mqtt::sendRegister: Registering UUID and actions with coordinator"));

  // Create a list of all registered actions
  DynamicJsonDocument actions_doc(JSON_ARRAY_SIZE(action_callbacks_.size()));
  JsonArray actions_array = actions_doc.to<JsonArray>();
  Serial.println(F("\tActions:"));

  for (const auto& action : action_callbacks_) {
    actions_array.add(action.first.c_str());
    Serial.println("\t\t" + action.first);
  }

  DynamicJsonDocument objects_doc(JSON_ARRAY_SIZE(object_callbacks_.size()));
  JsonArray objects_array = objects_doc.to<JsonArray>();
  Serial.println(F("\tObjects:"));
  
  for (const auto& object : object_callbacks_) {
    objects_array.add(object.first.c_str());
    Serial.println("\t\t" + object.first);
  }

  // Add the UUID entry
  DynamicJsonDocument doc(BB_MQTT_JSON_PAYLOAD_SIZE);
  doc["uuid"] = ESPRandom::uuidToString(getUuid());
  doc["actions"] = actions_array;
  doc["objects"] = objects_array;

  // Calculate the size of the resultant serialized JSON, create a buffer of
  // that size and serialize the JSON into that buffer.
  // Add extra byte for the null terminator
  std::vector<char> register_buf = std::vector<char>(measureJson(doc) + 1);
  size_t n = serializeJson(doc, register_buf.data(), register_buf.size());

  client_.publish("register", register_buf.data(), n);
}

void Mqtt::sendError(const String& who, const String& message,
                     const bool additional_serial_log) {
  String error = who;
  error += ": ";
  error += message;
  client_.publish(error_topic.c_str(), error.c_str());

  if (additional_serial_log == true) {
    Serial.println(error.c_str());
  }
}

void Mqtt::addAction(
    const String& name,
    std::function<void(char*, uint8_t*, unsigned int)> callback) {
  action_callbacks_[name] = callback;
}

void Mqtt::removeAction(const String& name) {
  action_callbacks_.erase(name.c_str());
}

void Mqtt::handleCallback(char* topic, uint8_t* message, unsigned int length) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);
  String topic_str(topic);

  // Handle action messages
  if (topic_str.startsWith(action_prefix_)) {
    // Extract the name from the topic and increment once to skip the last
    // slash: action/a48b109f-975f-42e2-9962-a6fb752a1b6e/pump -> pump
    char* action = strrchr(topic, '/');
    action++;
    auto it = action_callbacks_.find(action);

    if (it != action_callbacks_.end()) {
      it->second(topic, message, length);
    } else {
      sendError(who, String(F("No registered callback for action: ")) + action);
    }
    return;
  } else if (topic_str.startsWith(object_prefix_)) {
    library::Library::getLibrary().handleCallback(topic, message, length);
    return;
  }

  sendError(who, String(F("No registered callback for topic: ")) + topic);
}

const callback_map& Mqtt::getCallbackMap() { return action_callbacks_; }

}  // namespace bernd_box
