#ifndef BERND_BOX_CONFIG_H
#define BERND_BOX_CONFIG_H

// The prefix BB stands for Bernd Box

// MQTT
#define BB_MQTT_JSON_PAYLOAD_SIZE MQTT_MAX_PACKET_SIZE
#define BB_MQTT_TOPIC_ACTION_PREFIX "action"
#define BB_MQTT_TOPIC_OBJECT_PREFIX "object"
#define BB_MQTT_TOPIC_ADD_SUFFIX "add"
#define BB_MQTT_TOPIC_REMOVE_PREFIX "remove"
#define BB_MQTT_TOPIC_EXECUTE_PREFIX "execute"

#endif
