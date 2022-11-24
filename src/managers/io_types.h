#pragma once
#ifndef MINIMAL_BUILD

#include <ArduinoJson.h>
#include <BH1750.h>
#include <DallasTemperature.h>
#include <Max44009.h>
#include <SparkFunBME280.h>

#include <chrono>
#include <string>

namespace inamata {

/// I2C Connection
struct I2cConnection {
  int scl_pin;
  int sda_pin;
};

/// Sensor type of analog peripherials
struct AdcSensor {
  int pin_id;
  String name;
  float scaling_factor;
  String unit;
  int enable_pin_id;
};

/// Sensor type of Dallas temperature sensors
enum class DallasResolution { b9 = 9, b10 = 10, b11 = 11, b12 = 12 };

struct DallasSensor {
  DeviceAddress address;
  DallasResolution resolution;
  String name;
  String unit;
};

struct Measurement {
  float value;
  std::chrono::milliseconds timestamp;
  int sensor_id;
};

/// Sensor type of BH1750 light sensors
struct Bh1750Sensor {
  // Need copy-assignment due to GCC bug: stackoverflow.com/questions/53213681/
  void operator=(const Bh1750Sensor&) const {}
  BH1750 interface;
  uint8_t address;
  int i2c_interface_id;
};

/// Sensor type of MAX44009
struct Max44009Sensor {
  Max44009 interface;
  String name;
  String unit;
};

/// Sensor type of BME280 air sensors
enum class Bme280Parameter {
  kTemperatureC,
  kTemperatureF,
  kAltitudeFeet,
  kAltitudeMeters,
  kHumidity,
  kPressure,
};

struct Bme280Sensor {
  int address;
  Bme280Parameter parameter;
  String name;
  String unit;
};

struct L293dMotor {
  // Static parameters
  uint8_t pin_forward;  // When the pin is high, turn clockwise and vice versa.
  uint8_t pin_reverse;  // When the pin is low, turn clockwise and vice versa.
  uint8_t pin_enable;   // Pin to enable and control the motor's speed
};
}  // namespace inamata

#endif