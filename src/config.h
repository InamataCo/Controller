#ifndef BERND_BOX_CONFIG_H
#define BERND_BOX_CONFIG_H

// The prefix BB stands for Bernd Box

// MQTT
#define BB_MQTT_JSON_PAYLOAD_SIZE MQTT_MAX_PACKET_SIZE
#define BB_MQTT_PORT 1883
#define BB_MQTT_TOPIC_ACTION_PREFIX "action"
#define BB_MQTT_TOPIC_OBJECT_PREFIX "object"
#define BB_DEFAULT_QOS 1

#endif
