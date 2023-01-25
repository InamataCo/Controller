# ![Inamata Controller](./doc/images/header-logo.png)

This firmware interfaces with sensors and peripherals and connects to the server. It allows users to dynamically add and remove connected peripherals. The created peripherals can then be read from and commanded via the server.

## Supported Hardware

Below is a list of currently supported peripherals:

- All analog in / out peripherals
- All digital in / out peripherals
- All PWM peripherals
- All capacitive touch sensors
- BME/BMP280 air sensors
- NeoPixel LED strips

## Start Up

This is a controller's typical start up sequence:

1. Connect to one of the stored WiFi APs
2. Sync the local clock from an NTP server
3. Connect to the server via a WebSocket
4. Register its supported peripherals and task types with the server

## WebSocket Actions

Commands to create an LED and turn it on:

```json
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

## Formatting

The Google C++ code style is used. It is recommended to use clang-format to automatically format your code with the provided `.clang-format` file.

## Supported Hardware Details

### CSE7766 - Voltage, Current & Power

Status: TBD

Used to measure the current draw in the Athom SmartPlug V2

#### References

[ESPHome Doc](https://esphome.io/components/sensor/cse7766.html)
[ESPHome Code](https://github.com/esphome/esphome/blob/dev/esphome/components/cse7766/cse7766.cpp)
[Tasmota Code](https://github.com/erocm123/Sonoff-Tasmota/blob/master/sonoff/xnrg_02_cse7766.ino)
