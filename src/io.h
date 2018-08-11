/**
 * Read the analog inputs from the various sensors
 *
 * \author Moritz Ulmer <moritz.ulmer@posteo.de>
 * \date 2018
 * \copyright Apache License 2.0
 */

#ifndef BERND_BOX_IO_H
#define BERND_BOX_IO_H

#include <Arduino.h>
#include <BH1750.h>
#include <DallasTemperature.h>
#include <Max44009.h>
#include <OneWire.h>

// STD C++ includes placed after Arduino.h
#include <cmath>
#include <map>
#include <string>

namespace bernd_box {

/// Unique IDs of all connected sensors
enum class Sensor {
  kWaterTemperature,
  kSaturatedOxygen,
  kConductivity,
  kAciditiy,
  kTurbidity,
  kLightLevel,
  kLightLevel2,
  kLightLevel3,
  kUnknown
};

/// Sensor type of analog peripherials
struct AdcSensor {
  uint pin_id;
  std::string name;
  float scaling_factor;
  std::string unit;
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

class Io {
 public:
  // Pin to the status on-board LED
  const uint status_led_pin_ = 2;

  // List of connected Dallas temperature sensors
  const uint one_wire_pin_ = 3;
  std::map<Sensor, DallasSensor> dallases_ = {
      {{Sensor::kWaterTemperature}, {{0}, "water_temperature", "°C"}}};

  /// List of connected BH1750 and MAX44009 light sensors
  const uint i2c_scl_pin_ = 4;
  const uint i2c_sda_pin_ = 5;

  std::map<Sensor, Bh1750Sensor> bh1750s_ = {
      {{Sensor::kLightLevel},
       {BH1750::CONTINUOUS_LOW_RES_MODE, BH1750(0x23), "light_level", "lx"}},
      {{Sensor::kLightLevel2},
       {BH1750::ONE_TIME_LOW_RES_MODE, BH1750(0x5C), "light_level2", "lx"}}};

  std::map<Sensor, Max44009Sensor> max44009s_ = {
      {{Sensor::kLightLevel3}, {Max44009(0xCB), "light_level3", "lx"}}};

  /// List of connected analog peripherials
  const std::map<Sensor, AdcSensor> adcs_ = {
      {{Sensor::kSaturatedOxygen}, {32, "saturated_oxygen", 1.0, "SO2"}},
      {{Sensor::kConductivity}, {33, "conductivity", 1.0, "mS/cm"}},
      {{Sensor::kAciditiy}, {34, "acidity", 1.0, "pH"}},
      {{Sensor::kTurbidity}, {35, "turbidity", 1.0, "NTU"}},
      {{Sensor::kUnknown}, {36, "vn", 1.0, ""}},
      {{Sensor::kUnknown}, {36, "vn", 1.0, ""}}};

  Io() : one_wire_(one_wire_pin_) {}

  virtual ~Io() {}

  /**
   * Performs initialization of the connected sensors
   *
   * \return True if it was successful
   */
  bool init();

  /**
   * Turns the status LED on or off
   *
   * \param state True to turn the LED on
   */
  void setStatusLed(bool state);

  /**
   * Reads the value from the specified sensor
   *
   * \param sensor_id ID of the sensor to read out the value
   * \return Value read from the sensor, NAN if an error occured
   */
  float read(Sensor sensor_id);

  /**
   * Reads the analog value of the sensor with the given ID
   *
   * \param sensor_id The id of the sensor
   * \return Value in specified unit. NAN on error or not found
   */
  float readAnalog(Sensor sensor_id);

  /**
   * Gets the temperature
   *
   * \param sensor_id ID of the temperature sensor
   * \return Temperature in Celsius, NAN if sensor is not found
   */
  float readTemperature(Sensor sensor_id);

  /**
   * Gets the light level of the Bh1750 light sensors
   *
   * Error values can be 0 or 65536, but are also valid readings. To avoid
   * false-positives, don't check for these values.
   *
   * \param sensor_id ID of the light sensor
   * \return Light level in lux, NAN if an error occured
   */
  float readBh1750Light(Sensor sensor_id);

  /**
   * Gets the light level of the MAX44009 light sensors
   *
   * \param sensor_id ID of the light sensor
   * \return Light level in lux, NAN if an error occured
   */
  float readMax44009Light(Sensor sensor_id);

 private:
  /**
   * Checks if the sensor naming in the maps do not overlap
   *
   * \return True if they are valid
   */
  bool isSensorIdNamingValid();

  /// Returned when name of invalid sensor is requested
  const std::string empty_ = "";

  // Interface to Dallas temperature sensors
  OneWire one_wire_;
  DallasTemperature dallas_;
};

}  // namespace bernd_box

#endif