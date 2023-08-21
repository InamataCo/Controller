# Peripheral Configuration

[[_TOC_]]

## Peripheral List

The add command in the WebSocket API creates a new peripheral. Each add command
has to have the following JSON parameters in addition to peripheral specific
parameters. By default each peripheral type will only have a table to document
its constructor parameters. However, if it also supports parameters in its
capabilities, each table will be labelled as such.

| Parameter | Type   | Req. | Content                     |
| --------- | ------ | ---- | --------------------------- |
| uuid      | String | Yes  | unique ID of the peripheral |
| type      | String | Yes  | type of the peripheral      |

### Analog In

| Parameter               | Type   | Req. | Content                                          |
| ----------------------- | ------ | ---- | ------------------------------------------------ |
| pin                     | Number | Yes  | Analog input pin                                 |
| voltage_data_point_type | String | No   | Data point type for voltage readings (0V - 3.3V) |
| percent_data_point_type | String | No   | Data point type for percent readings (0 - 1)     |
| unit_data_point_type    | String | No   | Data point type for unit readings                |
| min_v                   | Number | No   | Minimum voltage (0V - 3.3V)                      |
| max_v                   | Number | No   | Maximum voltage (0V - 3.3V)                      |
| min_unit                | Number | No   | Minimum unit value                               |
| max_unit                | Number | No   | Maximum unit value                               |
| limit_unit              | Bool   | No   | Whether to clamp the mapped unit value           |

The voltage and percent data point type statically map the read analog value as
a voltage and percentage respectively. With the unit data point type, it is
possible to linearly map the voltage to another unit. For this, `min_v` and `max_v`
define the range on the voltage side, while `min_unit` and `max_unit` define the
target unit side. It is also possible to invert the gradient by using a `max_unit`
that is smaller than `min_unit`. By default the converted unit will be clamped
between the min and max values, but this can be disabled by setting `limit_unit`
to false.

On the ESP32 only the following pins can be used for analog measurements:

| Pin Name | Pin # |
| -------- | ----- |
| D32      | 32    |
| D33      | 33    |
| D34      | 34    |
| D35      | 35    |
| VN       | 36    |
| VP       | 39    |

If `unit_data_point_type` is set, `min_v`, `max_v`, `min_unit`, and `max_unit` have to be
set. `limit_unit` stays optional and is true by default.

### Analog Out

| Parameter               | Type   | Req. | Content                                         |
| ----------------------- | ------ | ---- | ----------------------------------------------- |
| pin                     | Number | Yes  | Analog output pin                               |
| voltage_data_point_type | String | No   | Data point type for setting voltage (0V - 3.3V) |
| percent_data_point_type | String | No   | Data point type for setting percent (0 - 1)     |

Either voltage or percent data point has to be defined.

### Atlas Scientific EC Meter (I2C)

#### Contstructor Parameters

| Parameter       | Type   | Req. | Content                          |
| --------------- | ------ | ---- | -------------------------------- |
| i2c_adapter     | String | Yes  | ID of the I2C adapter peripheral |
| i2c_address     | Number | Yes  | I2C address of the EC sensor     |
| data_point_type | String | Yes  | The data point type for EC       |
| probe_type      | String | Yes  | The type of probe used 10 - 0.01 |

#### Start Measurement Parameters

| Parameter     | Type   | Req. | Content                                             |
| ------------- | ------ | ---- | --------------------------------------------------- |
| temperature_c | Number | No   | Temperature (°C) to use for measurment compensation |

#### Start Calibration Parameters

| Parameter     | Type   | Req. | Content                                 |
| ------------- | ------ | ---- | --------------------------------------- |
| command       | String | Yes  | Which step in the calibration to run    |
| value         | Number | No   | The expected value to calibrate against |
| temperature_c | Number | No   | The water's temperature in celsius      |

The EC meter can be calibrated by a single or by two points. The controller task
that performs the calibration has to pass the command parameter as well as the
`value` and `temperature_c` parameters when required. The exact state
transitions are documented in `as_ec_meter.h`.

Valid `command` values are `clear`, `dry`, `single`, `double_low` and `double_high`.

### Atlas Scientific pH Meter

#### Contstructor Parameters

| Parameter       | Type   | Req. | Content                             |
| --------------- | ------ | ---- | ----------------------------------- |
| i2c_adapter     | String | Yes  | ID of the I2C adapter peripheral    |
| i2c_address     | String | Yes  | I2C address of the pH Meter         |
| data_point_type | String | Yes  | The data point type for pH (0 - 14) |

#### Start Measurement Parameters

| Parameter     | Type   | Req. | Content                                      |
| ------------- | ------ | ---- | -------------------------------------------- |
| temperature_c | Number | No   | Temperature (°C) for measurment compensation |

### Atlas Scientific RTD Meter

| Parameter       | Type   | Req. | Content                                  |
| --------------- | ------ | ---- | ---------------------------------------- |
| i2c_adapter     | String | Yes  | ID of the I2C adapter peripheral         |
| i2c_address     | String | Yes  | I2C address of the RTD Meter             |
| data_point_type | String | Yes  | The data point type for temperature (°C) |

### BME280 / BMP280 - Air Sensor

| Parameter                   | Type   | Req. | Content                                                |
| --------------------------- | ------ | ---- | ------------------------------------------------------ |
| i2c_adapter                 | String | Yes  | name of the I2C adapter peripheral                     |
| i2c_address                 | Number | Yes  | address of the BME / BMP280 sensor                     |
| temperature_data_point_type | String | Yes  | Data point type for temperature readings (0 - 60 °C)   |
| pressure_data_point_type    | String | Yes  | Data point type for pressure readings (300 - 1100 hPa) |
| humidity_data_point_type    | String | No   | Data point type for humidity readings (0 - 100 RH)     |

While adding the sensor, the type is checked (BME vs BMP) and if it is a BME
chip, the humidity data point type also has to be specified.

### Capacitive Sensor

| Parameter       | Type   | Req. | Content                               |
| --------------- | ------ | ---- | ------------------------------------- |
| sense_pin       | Number | Yes  | Capacitive sense input pin            |
| data_point_type | String | Yes  | Data point type for readings (0 or 1) |

Values close to 0 mean that touch was detected. The sensor is built into the
ESP32 and a wire only has to be connected to one of the compatible pins.

### CSE6677 - Power Sensor

| Parameter               | Type   | Req. | Content                                  |
| ----------------------- | ------ | ---- | ---------------------------------------- |
| uart_adapter            | String | Yes  | ID of the UART adapter peripheral        |
| voltage_data_point_type | String | Yes  | Data point type for voltage readings (V) |
| current_data_point_type | String | Yes  | Data point type for current readings (A) |
| power_data_point_type   | String | Yes  | Data point type for power readings (W)   |

### Digital In

| Parameter       | Type   | Req. | Content                               |
| --------------- | ------ | ---- | ------------------------------------- |
| pin             | Number | Yes  | Digital input pin                     |
| data_point_type | String | Yes  | Data point type for readings (0 or 1) |
| input_type      | String | Yes  | Pin mode configuration (Pullup/-down) |

For `input_type`, valid values are `floating`, `pullup` and `pulldown`.

### Digital Out

| Parameter       | Type   | Req. | Content                                     |
| --------------- | ------ | ---- | ------------------------------------------- |
| pin             | Number | Yes  | Digital output pin                          |
| data_point_type | String | Yes  | Data point type for setting state (0 or 1)  |
| initial_state   | Bool   | No   | Whether to set state when adding peripheral |
| active_low      | Bool   | No   | Whether to invert value                     |

### I2C Adapter

Configures the I2C interface and is used by other peripherals that communicate
via the I2C bus. For the ESP32 both I2C hardware units can be used.

| Parameter | Type   | Req. | Content          |
| --------- | ------ | ---- | ---------------- |
| scl       | Number | Yes  | clock signal pin |
| sda       | Number | Yes  | data signal pin  |

### NeoPixel

| Parameter      | Type   | Req. | Content                                      |
| -------------- | ------ | ---- | -------------------------------------------- |
| color_encoding | String | Yes  | The rgbw encoding used                       |
| led_pin        | Number | Yes  | The signalling pin connected to the NeoPixel |
| led_count      | Number | Yes  | The number of LEDs in the NeoPixel strip     |

The `color_encoding` is a string with a permutation of the `rgbw` characters.

### PWM

The peripheral supports the _SetValue_ capability for which _SetValue_ is the
simplest task to set a value. Expects a value between 0 and 1.

| Parameter       | Type   | Req. | Content                                            |
| --------------- | ------ | ---- | -------------------------------------------------- |
| pin             | Number | Yes  | PWM output pin                                     |
| data_point_type | String | Yes  | UUID of the data point type setting the brightness |

### UART Adapter

| Parameter | Type   | Req. | Content                                   |
| --------- | ------ | ---- | ----------------------------------------- |
| rx        | Number | Yes  | RX pin number                             |
| tx        | Number | Yes  | TX pin number                             |
| config    | String | Yes  | Config characters for encoding and parity |
| baud_rate | Number | Yes  | Baud rate of the connection               |

The `config` is expected in the format of `8N1` where the first char is the
number of data bits, the second the parity bit and the last the number of stop
bits.
