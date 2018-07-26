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

#include <array>
#include <cmath>
#include <string>

namespace bernd_box {

/**
 * List of connected analog peripherials
 */
struct AdcSensor {
  uint pin_id;
  std::string name;
  float scaling_factor;
  std::string unit;
};

class Io {
  // Pin to the status on-board LED
  const uint status_led_pin_ = 2;

  // TODO: verify the correct pin for the temperature 1-wire bus
  const uint one_wire_pin_ = 3;

  /// List of connected analog peripherials
  const std::array<AdcSensor, 6> adcs_ = {
      {{32, "saturated_oxygen", 1.0, "SO2"},  // Total dissolved oxygen
       {33, "conductivity", 1.0, "mS/cm"},    // Electrical conductivity
       {34, "acidity", 1.0, "pH"},            // Acidity
       {35, "turbidity", 1.0, "NTU"},         // Clarity of the water
       {36, "vn", 1.0, ""},                   // Some other parameter
       {39, "vp", 1.0, ""}}};                 // Some other parameter

 public:
  Io() : one_wire_(one_wire_pin_) { pinMode(status_led_pin_, OUTPUT); }

  virtual ~Io() {}

  void setStatusLed(bool state) {
    if (state == true) {
      digitalWrite(status_led_pin_, HIGH);
    } else {
      digitalWrite(status_led_pin_, LOW);
    }
  }

  /**
   * Reads the analog value of the sensor with the given ID
   *
   * \param sensor_id The id of the sensor
   * \return Value in specified unit. NAN on error
   */
  float read(uint sensor_id) {
    if (sensor_id < adcs_.size()) {
      float value = analogRead(adcs_[sensor_id].pin_id);
      value *= adcs_[sensor_id].scaling_factor;
      return value;
    } else {
      return NAN;
    }
  }

  /**
   * Returns the number of sensors
   *
   * \return The sensor count
   */
  uint getSensorCount() const { return adcs_.size(); }

  /**
   * Gets the name of the sensor with the sensor ID
   *
   * \param sensor_id The id of the sensor
   * \return The name of the sensor. Empty string on error
   */
  const std::string& getSensorName(uint sensor_id) const {
    if (sensor_id < adcs_.size()) {
      return adcs_[sensor_id].name;
    } else {
      return empty_;
    }
  }

  /**
   * Gets the ID of the pin with sensor ID
   *
   * \param sensor_id The id of the sensor
   * \return The ID of the pin. -1 on error
   */
  int getSensorPin(uint sensor_id) const {
    if (sensor_id < adcs_.size()) {
      return adcs_[sensor_id].pin_id;
    } else {
      return -1;
    }
  }

  /**
   * Gets the unit of the measurement for the given sensor ID
   *
   * \param sensor_id The id of the sensor
   * \return The unit used by the sensor. Empty string on error
   */
  const std::string& getSensorUnit(uint sensor_id) const {
    if (sensor_id < adcs_.size()) {
      return adcs_[sensor_id].unit;
    } else {
      return empty_;
    }
  }

 private:
  /// Returned when name of invalid sensor is requested
  const std::string empty_ = "";

  // Setup a oneWire instance to communicate with any OneWire devices
  OneWire one_wire_;

  // Interface to Dallas temperature sensors
  DallasTemperature dallas_;
};

}  // namespace bernd_box

#endif