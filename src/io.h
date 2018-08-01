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
  bool init() {
    bool success = true;
    // Set the status LED GPIO to output
    pinMode(status_led_pin_, OUTPUT);

    // Start Wire for the BH1750 light sensors, then set the sense mode
    Wire.begin(i2c_sda_pin_, i2c_scl_pin_);
    for (auto& it : bh1750s_) {
      it.second.interface.begin(it.second.mode);
    }

    // Start up the Dallas Temperature library
    dallas_.begin();
    // locate devices on the bus
    uint dallas_count = dallas_.getDeviceCount();
    Serial.printf("Found %d temperature senors\n", dallas_count);
    if (dallas_count == dallases_.size()) {
      uint index = 0;
      for (auto it = dallases_.begin(); it != dallases_.end(); ++it) {
        dallas_.getAddress(it->second.address, index);
        index++;
      }
    } else {
      Serial.printf("Expected to find %d sensors\n", dallases_.size());
      success = false;
    }

    // Check if no sensor IDs are reused among different types
    if (!isSensorIdNamingValid()) {
      success = false;
    }

    return success;
  }

  /**
   * Turns the status LED on or off
   *
   * \param state True to turn the LED on
   */
  void setStatusLed(bool state) {
    if (state == true) {
      digitalWrite(status_led_pin_, HIGH);
    } else {
      digitalWrite(status_led_pin_, LOW);
    }
  }

  /**
   * Reads the value from the specified sensor
   *
   * \param sensor_id ID of the sensor to read out the value
   * \return Value read from the sensor, NAN if an error occured
   */
  float read(Sensor sensor_id) {
    // If no matching sensors are found, return NAN
    float value = NAN;

    // If no sensor has been found, check analog sensors
    if (value == NAN) {
      value = readAnalog(sensor_id);
    }

    // If no sensor has been found, check the temperature sensors
    if (value == NAN) {
      value = readTemperature(sensor_id);
    }

    // If no sensor has been found, check the light sensors
    if (value == NAN) {
      value = readBh1750Light(sensor_id);
    }

    // If no sensor has been found, check the light sensors
    if (value == NAN) {
      value = readMax44009Light(sensor_id);
    }

    return value;
  }

  /**
   * Reads the analog value of the sensor with the given ID
   *
   * \param sensor_id The id of the sensor
   * \return Value in specified unit. NAN on error or not found
   */
  float readAnalog(Sensor sensor_id) {
    float value = NAN;

    auto it = adcs_.find(sensor_id);
    if (it != adcs_.end()) {
      value = analogRead(it->second.pin_id);
      value *= it->second.scaling_factor;
    }

    return value;
  }

  /**
   * Gets the temperature
   *
   * \param sensor_id ID of the temperature sensor
   * \return Temperature in Celsius, NAN if sensor is not found
   */
  float readTemperature(Sensor sensor_id) {
    float temperature_c = NAN;

    auto it = dallases_.find(sensor_id);

    // If the sensor was found in the registered list
    if (it != dallases_.end()) {
      temperature_c = dallas_.getTempC(it->second.address);

      // If the value is invalid, print its ID and address
      if (temperature_c == DEVICE_DISCONNECTED_C) {
        Serial.printf("Dallas sensor (%d) not connected: ",
                      static_cast<uint>(sensor_id));
        for (uint i = 0; i < sizeof(DeviceAddress); i++) {
          Serial.print(it->second.address[i]);
        }
        Serial.println();
        temperature_c = NAN;
      }
    }

    return temperature_c;
  }

  /**
   * Gets the light level of the Bh1750 light sensors
   *
   * Error values can be 0 or 65536, but are also valid readings. To avoid
   * false-positives, don't check for these values.
   *
   * \param sensor_id ID of the light sensor
   * \return Light level in lux, NAN if an error occured
   */
  float readBh1750Light(Sensor sensor_id) {
    float lux = NAN;

    const auto& it = bh1750s_.find(sensor_id);

    // If the sensor was found in the registered list
    if (it != bh1750s_.end()) {
      lux = it->second.interface.readLightLevel();
    }

    return lux;
  }

  /**
   * Gets the light level of the MAX44009 light sensors
   *
   * \param sensor_id ID of the light sensor
   * \return Light level in lux, NAN if an error occured
   */
  float readMax44009Light(Sensor sensor_id) {
    float lux = NAN;

    const auto& it = max44009s_.find(sensor_id);

    // If the sensor was found in the registered list
    if (it != max44009s_.end()) {
      lux = it->second.interface.getLux();
      int error = it->second.interface.getError();
      if (error != 0) {
        Serial.printf("MAX44009 error while reading: %d\n", error);
      }
    }

    return lux;
  }

 private:
  /**
   * Checks if the sensor naming in the maps do not overlap
   *
   * \return True if they are valid
   */
  bool isSensorIdNamingValid() {
    bool valid = true;

    // Compare adcs with dallases and bh1750s
    for (auto& i : adcs_) {
      for (auto& j : dallases_) {
        if (i.first == j.first) {
          valid = false;
          Serial.printf("Same ID (%d) used by adcs and dallases\n",
                        static_cast<uint>(i.first));
        }
      }
      for (auto& j : bh1750s_) {
        if (i.first == j.first) {
          valid = false;
          Serial.printf("Same ID (%d) used by adcs and bh1750s\n",
                        static_cast<uint>(i.first));
        }
      }
    }

    // Compare dallases_ with bh1750s
    for (auto& i : dallases_) {
      for (auto& j : bh1750s_) {
        if (i.first == j.first) {
          valid = false;
          Serial.printf("Same ID (%d) used by dallases and bh1750s\n",
                        static_cast<uint>(i.first));
        }
      }
    }
    return valid;
  }

  /// Returned when name of invalid sensor is requested
  const std::string empty_ = "";

  // Interface to Dallas temperature sensors
  OneWire one_wire_;
  DallasTemperature dallas_;
};

}  // namespace bernd_box

#endif