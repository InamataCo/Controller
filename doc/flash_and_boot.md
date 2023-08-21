[[_TOC_]]

# Boot Sequence

## ESP32

On power on, the ESP32 runs its ROM based first stage bootloader. This hands over to the 2nd stage bootloader (at <span dir="">0x1000</span>). It uses the partition table (at <span dir="">0x8000</span>) to find the otadata partition. The data in this partition is used to select which app (firmware) slot to execute (0 or 1). Using the partition table, it finds the entry point to the app slot and hands over execution to it. The app is the compiled and packaged Arduino-based program.

## ESP8266

On power on, the ESP8266 runs its ROM based first stage bootloader. This hands over to the 2nd stage bootloader that was compiled into the firmware image. It uses the partition table (also compiled into the image) to find the data regarding which app (firmware) slot to execute (0 or 1). Using that information, it hands over execution to it.

## Differences Between the ESP32 and ESP8266 Platforms

For the ESP8266, the 2nd stage bootloader and partition tables are compiled into the firmware image. In the ESP32's case, they are flashed separately. This means that the Flasher has to download the partition table and bootloader for the ESP32 in addition to its firmware, where as for the ESP8266 only the firmware is required. This means however, that supporting different flash sizes and layouts is easier for the ESP32, since only the partition table has to be customized instead of compiling a new image. This has the effect, that a larger number of ESP8266 firmware lines have to be maintained as this complexity is stored in them. This is especially true as different ESP8266 boards need different bootloaders depending on the flash interface (DIO, QIO, QOUT, DOUT).

# File System

LittleFS is the file system used by both platforms. During the flash process, secrets as well as the TLS certificate of the root CA as saved there. However, when performing an OTA update via a controller's own WiFi network, the certificate has to be included in the firmware.

The TLS certificate is required as the WebSockets connection uses TLS encryption and verify the server's certificate. The current certificate is valid until 22.06.2036. Before this time point, an update mechanism has to be implemented to fetch the new certificate. A possible approach would be to handle this when a controller connects, to send a command to download a new cert.

The choice of where to store CA certs differs for the ESP32 and ESP8266. Option A is in the file system and load it into memory during boot or compiled into the firmware itself. The ESP32 [transparently maps](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/memory-types.html#drom-data-stored-in-flash) const char arrays from flash into memory through a cache, thereby not using DRAM (data RAM). The ESP8266 however will [place it into RAM](https://arduino-esp8266.readthedocs.io/en/latest/PROGMEM.html), unless PROGMEM / F() / FSPTR() are used. Since the WiFiClientSecure class only accepts a const char array (and not an F-String) for the certificate, it has to be loaded into RAM either way.

However, the question of how to handle multiple certificates is open. This is required when the existing certificate expires or is revoked and has to be replaced by a new one. Approaches here can store multiple certificate files in the file system or combine them into a single file. The question of selecting between them is also open.

## secrets.json

The secrets configuration is stored on the flash either in `secrets.json` for ESP32's or the EEPROM section for ESP8266 variants. In ESP8266's the first 4 bytes are used for the size of the serialized JSON followed by the JSON itself. Below is an example of the secrets JSON.

```
{
  "wifi_aps": [
    { "ssid": "Mindspace2.4", "password": "HelloThere" },
    { "ssid": "HasenbauOben", "password": "" },
    { "ssid": "Berge24", "password": "EinPasswort" }
  ],
  "ws_token": "d60f9b8fb0474649b584fa897e76394f272bdc36",
  "core_domain": "core.staging.inamata.co",
  "secure_url": true,
  "name": "Controller 3"
}
```

# On-Device Encryption

The [Espressif docs](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/security/flash-encryption.html) give a comprehensive overview of flash encryption. This should be used in combination with secure boot which ensures that only signed binaries are executed.

When using flash encryption in production mode, it is only possible to update the firmware via OTA updates, USB flashing is disabled. The encryption itself is transparent to reading/writing the data from the firmware itself, which means that it doesn't require any code changes. Additional material to help enabling it are listed below:

- https://community.platformio.org/t/protect-arduino-sketch-on-esp32-via-flash-encryption/12665/5
- https://github.com/pedros89/ESP32-update-both-OTA-Firmware-and-OTA-SPIFFS-with-Arduino