/**
 * Read the inputs from the various sensors
 *
 * \author Moritz Ulmer <moritz.ulmer@posteo.de>
 * \date 2018
 * \copyright Apache License 2.0
 */

#ifndef BERND_BOX_IO_H
#define BERND_BOX_IO_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "managers/mqtt.h"
#include "managers/services.h"

// STD C++ includes placed after Arduino.h
#include <algorithm>
#include <cmath>
#include <map>
#include <string>

#include "io_types.h"

namespace bernd_box {

class Io {
 public:
  /// Pin to the status on-board LED
  const uint status_led_pin_ = 2;

  /// Pin to the pump
  const uint pump_pin_ = 27;

  /// Pin to power on the ATX power supply
  const uint atx_power_pin_ = 14;

  /// Motor drivers to power motors
  std::map<int, L293dMotor> l293d_motors_;

  // List of connected Dallas temperature sensors (DS18B20)
  const uint one_wire_pin_ = 27;
  std::map<int, DallasSensor> dallases_ = {
      // {{Sensor::kWaterTemperature},
      //  {{0}, DallasResolution::b12, "water_temperature", "°C"}},
  };

  /// List of connected BH1750 and MAX44009 light sensors
  const uint i2c_scl_pin_ = 22;
  const uint i2c_sda_pin_ = 21;

  /// List of connected BME280 sensor paramters
  const std::map<int, Bme280Sensor> bme280s_ = {}; /*
      {{Sensor::kAirTemperature},
       {0x77, Bme280Parameter::kTemperatureC, "air_temperature", "°C"}},
      {{Sensor::kAirPressure},
       {0x77, Bme280Parameter::kPressure, "air_pressure", "Pa"}},
      {{Sensor::kAirHumidity},
       {0x77, Bme280Parameter::kHumidity, "air_humidity", "%"}},
  };*/
  /// List of connected BME280 sensors
  std::map<uint, BME280> bme280sensors_ = {/*{{0x77}, {BME280()}}*/};

  /// List of BH1750 sensors
  std::map<int, Bh1750Sensor> bh1750s_ = {}; /*
       {{Sensor::kLightLevel},
        {BH1750::CONTINUOUS_LOW_RES_MODE, BH1750(0x23), "light_level", "lx"}},
       {{Sensor::kLightLevel2},
        {BH1750::ONE_TIME_LOW_RES_MODE, BH1750(0x5C), "light_level2", "lx"}},
   };*/

  std::map<int, Max44009Sensor> max44009s_ = {}; /*
      {{Sensor::kLightLevel3}, {Max44009(0x4A), "ambient_brightness", "lx"}},
  };*/

  /// List of connected analog peripherials
  const std::map<int, AdcSensor> adcs_ = {
      // {{Sensor::kTurbidity}, {35, "turbidity", 1.0, "NTU", -1}},
      // {{Sensor::kAciditiy}, {32, "acidity", 1.0, "pH", 33}},
      // {{Sensor::kTotalDissolvedSolids},
      //  {39, "total_dissolved_solids", 1.0, "mg/l", 26}},
      // {{Sensor::kDissolvedOxygen}, {36, "dissolved_oxygen", 1.0, "SO2", 5}},
      // {{Sensor::kConductivity}, {34, "conductivity", 1.0, "mS/cm", 17}},
  };
  const uint analog_raw_range_ = 4096;
  const float analog_reference_v_ = 3.3;

  Io(Mqtt& mqtt);

  virtual ~Io();

  /**
   * Performs initialization of the connected sensors
   *
   * \return True if it was successful
   */
  Result init();

  /**
   * Create a device name and get an ID
   *
   * \param name Name of the device, has to be unique
   * \param id The resulting ID is saved into the reference. No change on error
   * \return kSuccess on success, kNameAlreadyExists if the name is taken
   */
  Result addDevice(const String& name, int& id);

  /**
   * Create a device name and get an ID
   *
   * \param name Name of the device, has to be unique
   * \param id The resulting ID is saved into the reference
   * \return kSuccess on success, kNameAlreadyExists if the name is taken
   */
  void removeDevice(const int id);

  /**
   * Get a device name for a given ID
   *
   * \param id The device's ID
   * \return A reference to the device's name
   */
  const String& getName(const int id);

  /**
   * Gets the ID of a device for a given name
   *
   * \param name The device's name
   * \return The ID
   */
  const int getId(const String& name);

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
  float read(int sensor_id);

  /**
   * Reads the raw value of the sensor with the given ID
   *
   * \param sensor_id The id of the sensor
   * \return Value in specified unit. NAN on error or not found
   */
  float readAnalog(int sensor_id);

  /**
   * Reads the voltage of the sensor with the given ID
   *
   * \param sensor_id The id of the sensor
   * \return Value in specified unit. NAN on error or not found
   */
  float readAnalogV(int sensor_id);

  Result enableAnalog(int sensor_id);
  Result enableAnalog(const AdcSensor& adc);
  void enableAllAnalog();

  void disableAnalog(int sensor_id);
  void disableAnalog(const AdcSensor& adc);
  void disableAllAnalog();

  Result setPumpState(bool state);

  void enableOutput(uint8_t pin);
  void setPinState(uint8_t pin, bool state);

  /**
   * Set a PWM signal on a pin
   *
   * Assigns one of 16 PWM generators to the specified pin. Dissasociates it
   * when removePinPwm is called.
   *
   * \param pin The pin number
   * \param percent Percentage from 0 - 100
   * \return kSuccess on success, kFailure otherwise
   */
  Result setPinPwm(uint8_t pin, float percent);
  void removePinPwm(uint8_t pin);

  /**
   * Gets the temperature in sync mode (~800ms blocking)
   *
   * \param sensor_id ID of the temperature sensor
   * \return Temperature in Celsius, NAN if sensor is not found
   */
  float readDallasTemperature(int sensor_id);

  /**
   * Requests the selected Dallas sensor to acquire the current temperature
   *
   * The duration of the process depends on the set resolution (100ms - 800ms)
   *
   * \param sensor The one-wire address of the Dallas sensor
   * \param millisToWait Returns the expected wait time depending on the
   * resolution
   * \return kSuccess on success, kDeviceDisconnected if the sensor could not be
   * found
   */
  Result requestDallasTemperatureUpdate(const DallasSensor& sensor,
                                        int& millisToWait);

  /**
   * Returns the temperature in °C is possible from selected sensor
   *
   * On error, the temperature_c variable will be set to NAN.
   *
   * \param sensor The sensor from which the temperature should be read
   *
   * \param asyncMode If true, requestTemperatureUpdate() has to have been
   * called and the appropriate time waited. It will not block. If it is false,
   * it will block until the sensor has acquired the temperature. This can take
   * up to 800ms.
   *
   * \param temperature_c The variable
   * where the temperature will be returned
   *
   * \return kSuccess on success, kNotReady if the read was too soon after the
   * request, kDeviceDisconnected if the device could not be found
   */
  Result readDallasTemperature(const DallasSensor& sensor, bool asyncMode,
                               float& temperature_c);

  /**
   * Converts the 8 byte Dallas DeviceAddress uint8_t array to a 64bit integer
   *
   * \param address Pointer to the 8 byte uint8_t array
   * \return Integer representation of the address
   */
  long dallasAddressToInt(const uint8_t* address);

  /**
   * Gets the light level of the Bh1750 light sensors
   *
   * Error values can be 0 or 65536, but are also valid readings. To avoid
   * false-positives, don't check for these values.
   *
   * \param sensor_id ID of the light sensor
   * \return Light level in lux, NAN if an error occured
   */
  float readBh1750Light(int sensor_id);

  /**
   * Gets the light level of the MAX44009 light sensors
   *
   * \param sensor_id ID of the light sensor
   * \return Light level in lux, NAN if an error occured
   */
  float readMax44009Light(int sensor_id);

  float readBme280Air(int sensor_id);

  /**
   * Checks if a client with the address exists on an I2C bus
   * 
   * \param address Address of the client to check
   * \return Device ID of the I2C bus
   */
  Result checkI2cAddress(uint8_t address, const int i2c_interface_id);

  const std::map<int, TwoWire&>& getI2cInterfaces();

 private:
  void i2cMqttCallback(char* topic, uint8_t* payload, unsigned int length);
  Result addI2cInterface(const JsonObjectConst& doc);
  Result removeI2cInterface(const JsonObjectConst& doc);

  Mqtt& mqtt_;

  /// Mapping of 16 PWM channels to pins. Index = channel, value = pin
  /// pwm_channel_[2] = 33; --> pin 33 is using pwm channel 2
  /// -1 means the channel is not being used
  std::array<int8_t, 16> pwm_channels_;

  /// The higher the frequency, the lower the resolution. 5 kHz is the lowest
  /// and results in 13 bits of resultion.
  /// https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/ledc.html
  const uint32_t pwm_frequency_ = 5000;
  const uint8_t pwm_resolution_ = 13;

  /// Returned when name of invalid sensor is requested
  const String empty_ = "";

  /// Unique identifier for all IO devices (sensors and actuators). The ID is
  /// used as an internal identifier. It's name is used for communication with
  /// external devices. An ID of -1 is used for invalid IDs
  int last_name_id_ = -1;
  std::map<int, String> names_;

  // Interface to Dallas temperature sensors
  OneWire one_wire_;
  DallasTemperature dallas_;

  // List of I2C interfaces
  std::map<int, TwoWire&> i2c_interfaces_;
};

}  // namespace bernd_box

#endif
