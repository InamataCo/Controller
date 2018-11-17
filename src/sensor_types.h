#ifndef BERND_BOX_SENSOR_TYPES_H
#define BERND_BOX_SENSOR_TYPES_H

#include <BH1750.h>
#include <DallasTemperature.h>
#include <Max44009.h>
#include <SparkFunBME280.h>

#include <string>

namespace bernd_box {

/// Unique IDs of all connected sensors
enum class Sensor {
  kWaterTemperature,
  kDissolvedOxygen,
  kConductivity,
  kAciditiy,
  kTurbidity,
  kTotalDissolvedSolids,
  kLightLevel,
  kLightLevel2,
  kLightLevel3,
  kAirTemperature,
  kAirPressure,
  kAirHumidity,
  kUnknown
};

/// I2C Connection
struct I2cConnection {
  int scl_pin;
  int sda_pin;
};

/// Sensor type of analog peripherials
struct AdcSensor {
  int pin_id;
  std::string name;
  float scaling_factor;
  std::string unit;
  int enable_pin_id;
};

/// Sensor type of Dallas temperature sensors
struct DallasSensor {
  DeviceAddress address;
  std::string name;
  std::string unit;
};

/// Sensor type of BH1750 light sensors
struct Bh1750Sensor {
  BH1750::Mode mode;
  BH1750 interface;
  std::string name;
  std::string unit;
};

/// Sensor type of MAX44009
struct Max44009Sensor {
  Max44009 interface;
  std::string name;
  std::string unit;
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
  std::string name;
  std::string unit;
};

}  // namespace bernd_box

#endif