# SDG Controller

## Start Up

This is a controller's typical start up sequence ([serial output](#start-up-output)):

1. Read (or generate if unavailable) the stored UUID
2. Connect to one of the stored WiFi APs
3. Sync the local clock from an NTP server
4. Connect to the SDG server via a WebSocket
5. Register its supported peripherals and task types with the SDG server

## WebSocket Actions

Commands to create an LED and turn it on:

```
{
  "type": "cmd",
  "peripheral": {
    "add": [
      {
        "name": "led_builtin",
        "type": "LED",
        "pin": 2
      }
    ]
  }
}


{
  "type": "cmd",
  "task": {
    "create": [
      {
        "type": "WriteActuator",
        "peripheral_name": "led_builtin",
        "value": 1,
        "unit": "%"
      }
    ]
  }
}
```

## MQTT Actions (Depreciated, for now)

Actions are commands that can be sent via MQTT for the controllers to perform. It is formatted as:

    action/<UUID>/<action-name>
    action/a48b109f-975f-42e2-9962-a6fb752a1b6e/l293d_motor

The UUID specifies which controller should execute the action and the action name specifies which action should be executed. The payload is JSON formatted and an example for the pump action would be:

    {"enable": {"name": "main_pump","power_percent": 44.3, }}

This tells the pump to activate for 40 seconds.

## Formatting

The Google C++ code style is used. It is recommended to use clang-format to automatically format your code with the provided `.clang-format` file.

Temperature is 1-wire interface

## Appendix

### Start Up Output

    eae5aaf9-bf92-414e-8fe8-78a893ee05f7
    Network::connect: Searching for the following networks:
            SDGintern
            PROTOHAUS
            Connected! IP address is 192.168.0.50
    Network::setClock: Waiting for NTP time sync
            .
    Current time: Tue Sep 22 23:31:40 2020
    WebSocket::HandleEvent: Connected to url: /ws-api/v1/farms/controllers/
    Peripheral types:
            CapacitiveSensor
            DummyPeripheral
            InvalidPeripheral
            LED
            NeoPixel
    Task types:
            AlertSensor
            DummyTask
            ReadSensor
            SetLight
            WriteActuator
    CheckConnectivity: Reconnected to server
