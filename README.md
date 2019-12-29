# Bernd Box

## Start Up

This is a controller's typical start up sequence ([serial output](#start-up-output)):

1. Read (or generate if unavailable) the stored UUID
2. Connect to WiFi
3. Ping the SDG server with its UUID or MAC 
4. Get the local IP address of the local coordinator
5. Sync the local clock from an NTP server
6. Connect to the coordinator's MQTT broker
7. Register itself and its actions

## Registration

The controller sends a JSON doc to the coordinator after connecting to its MQTT broker containing its

- UUID
- Action list

## Actions

Actions are commands that can be sent via MQTT for the controllers to perform. It is formatted as:

    action/<UUID>/<action-name>
    action/a48b109f-975f-42e2-9962-a6fb752a1b6e/pump

The UUID specifies which controller should execute the action and the action name specifies which action should be executed. The payload is JSON formatted and an example for the pump action would be:

    {"duration_s": 40,"enable": true}

This tells the pump to activate for 40 seconds.

## Formatting

The Google C++ code style is used. It is recommended to use clang-format to automatically format your code with the provided `.clang-format` file.

Temperature is 1-wire interface

## Appendix

### Start Up Output

    a48b109f-975f-42e2-9962-a6fb752a1b6e
    WiFi: Disconnected. Attempting to reconnect
    WiFi: Attempting to connect to HasenbauOben
    .
    WiFi: Connected! IP address is 192.168.1.8
    Send ping to: https://core.staging.openfarming.ai/api/v1/farms/controllers/ping/
    {"uuid":"a48b109f-975f-42e2-9962-a6fb752a1b6e","wifi_mac_address":"30:AE:A4:45:C1:60"}
    HTTP response code: 201
    {"id": "6de4a1cb-8ff6-4d10-80e4-ba760240991c", "name": null, "wifi_mac_address": "30-AE-A4-45-C1-60", "external_ip_address": "112.134.232.78", "controller_type": "UNK"}

    Waiting for NTP time sync: 
    Current time: Wed Dec 25 13:26:34 2019
    Send ping to: https://core.staging.openfarming.ai/api/v1/farms/controllers/ping/
    {"coordinator_local_ip_address": "192.168.1.102"}
    Coordinator's local IP address: 192.168.1.102
    192.168.1.102:1883
    MQTT connecting to 192.168.1.102. Try #1
    MQTT: Connected!
    List of registered actions:
            pump
