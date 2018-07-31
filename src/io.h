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
#include <DallasTemperature.h>
#include <OneWire.h>

#include <cmath>
#include <map>
#include <string>

namespace bernd_box {

enum class Sensor {
  kWaterTemperature,
  kSaturatedOxygen,
  kConductivity,
  kAciditiy,
  kTurbidity,
  kLightLevel,
  kUnknown
};

/**
 * List of connected analog peripherials
 */
struct AdcSensor {
  uint pin_id;
  std::string name;
  float scaling_factor;
  std::string unit;
};

struct DallasSensor {
  DeviceAddress address;
  std::string name;
  std::string unit;
};

class Io {
 public:
  // Pin to the status on-board LED
  const uint status_led_pin_ = 2;

  // TODO: verify the correct pin for the temperature 1-wire bus
  const uint one_wire_pin_ = 3;
  std::map<Sensor, DallasSensor> dallases_ = {
      {{Sensor::kWaterTemperature}, {{0}, "water_temperature", "°C"}}};

  // const i2c_slk_pin_ = 4;
  // const i2c_sda_pin_ = 5;

  // BH1750 lightMeter(0x23);
  // BH1750 lightMeter_2(0x5C);
  // Max44009 myLux(0x4A);  // default addr

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

  bool init() {
    bool success = true;
    // Set the status LED GPIO to output
    pinMode(status_led_pin_, OUTPUT);

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

  float read(Sensor sensor_id) {
    // If no matching sensors are found, return NAN
    float value = NAN;

    // If no sensor has been found, check analog sensors
    if (value == NAN) {
      value = readAnalog(sensor_id);
    }

    if (value == NAN) {
      value = readTemperature(sensor_id);
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
   * \param sensor_id ID of the temperature sensor, defaults to the first one
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
    } else {
      Serial.printf("Sensor not registered: %d\n",
                    static_cast<uint>(sensor_id));
    }

    return temperature_c;
  }

 private:
  /// Returned when name of invalid sensor is requested
  const std::string empty_ = "";

  // Interface to Dallas temperature sensors
  OneWire one_wire_;
  DallasTemperature dallas_;
};

}  // namespace bernd_box

#endif