Registration topics:
- fl_box/regisration/v1/hello
```
Sample for the Hardware State JSON message
{
  "macAddress": "01:02:03:04:05:06",
  "type": "Bernd Box",
  "dallases": [
    {"address": 1, "name": "water_temperature", "unit": "°C", "state": "active"},
    {"address": 2, "name": "water_temperature", "unit": "°C", "state": "failed"}
  ],
  "bme280sensors": [
    {
      "address": 0x77,
      "parameters": [
        {"name": "air_temperature", "unit": "°C"},
        {"name": "air_pressure", "unit": "Pa"},
        {"name": "air_humidity", "unit": "°C"}
      ],
      "state": "unknown",
    },
    {
      "address": 0x76,
      "parameters": [
        {"name": "air_temperature", "unit": "°C"},
        {"name": "air_pressure", "unit": "Pa"},
        {"name": "air_humidity", "unit": "°C"}
      ],
      "state": "unknown"
    }
  ],
  "bh1750s": [
    {"address": 0x4a, "name": "ambient_brightness", "unit": "lx", "state": "active"},
    {"address": 0x4b, "name": "ambient_brightness", "unit": "lx", "state": "active"}
  ],
  "adcs": [
    {"pin": 32, "name": "saturated_oxygen", "unit": "SO2", "state": "unknown"},
    {"pin": 33, "name": "conductivity", "unit": "SO2", "state": "unknown"},
    {"pin": 34, "name": "acidity", "unit": "pH", "sample_count": 30, "state": "unknown"},
    {"pin": 34, "name": "turbidity", "unit": "NTU", "state": "unknown"}
  ],
  "leds": [
    {"pin": 2, "name": "active"}
  ]
}
```
    
Registration:
- Box sends MAC address on `hello` every 1s
- NodeRED displays that a new device is available
