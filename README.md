![Inamata logo](doc/images/inamata-logo.png)

# Inamata Controller

This firmware interfaces with sensors and peripherals and connects to the server. It allows users to dynamically add and remove connected peripherals. The created peripherals can then be read from and commanded via the server. The firmware can be flashed with the [Flasher][1] or via PlatformIO in VS Code.

[[_TOC_]]

## Supported Hardware

Below is a list of currently supported peripherals in the firmware. Some may not yet be supported by the web app:

- All analog in / out peripherals
- All digital in / out peripherals
- All PWM peripherals
- All capacitive touch sensors
- BME/BMP280 air sensors
- CSE6677 power sensor
- NeoPixel LED strips
- Atlas Scientific EC / pH / RTD meters

The [peripherals page](doc/peripherals.md) lists additional details for each type.

## Start Up

This is a controller's typical start up sequence:

1. Connect to one of the stored WiFi APs
2. Sync the local clock from an NTP server
3. Connect to the server via a WebSocket
4. Register its supported peripherals and task types with the server

## WiFi AP Setup

1. Power on the controller
2. Connect to the InamataSetup WiFi
3. Your browser should open the captive portal on 192.168.4.1 (check not https)
4. Press configure WiFi and enter the details:
   - Select WiFi name
   - Enter WiFi password
   - Enter the controller auth key
   - For local dev enter your IP such as `192.168.1.3`. The port used is 8000
   - For local dev enter `n` to use HTTP instead of HTTPS
5. Press save and possibly reboot the device

## WebSocket Actions

The WebSocket protocol is described on the [WebSocket API page](doc/websocket_api.md).

## Development

Development is mainly done in a private GitLab repository but changes are automatically pushed to this public GitHub repository.

## Formatting

The Google C++ code style is used. It is recommended to use clang-format to automatically format your code with the provided `.clang-format` file.

#### References

- [ESPHome Doc](https://esphome.io/components/sensor/cse7766.html)
- [ESPHome Code](https://github.com/esphome/esphome/blob/dev/esphome/components/cse7766/cse7766.cpp)
- [Tasmota Code](https://github.com/erocm123/Sonoff-Tasmota/blob/master/sonoff/xnrg_02_cse7766.ino)

[1]: https://github.com/InamataCo/Flasher