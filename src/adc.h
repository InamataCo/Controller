/**
 * Read the analog inputs from the various sensors
 *
 * \author Moritz Ulmer <moritz.ulmer@posteo.de>
 * \date 2018
 * \copyright Apache License 2.0
 */

#ifndef BERND_BOX_ADC_H
#define BERND_BOX_ADC_H

#include <Arduino.h>
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

class Adc {
 public:
  /**
   * Reads the analog value of the sensor with the given ID
   *
   * \param sensor_id The id of the sensor
   * \return Value in specified unit. NAN on error
   */
  float read(uint sensor_id) {
    if (sensor_id < elements_.size()) {
      float value = analogRead(elements_[sensor_id].pin_id);
      value *= elements_[sensor_id].scaling_factor;
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
  uint getSensorCount() const { return elements_.size(); }

  /**
   * Gets the name of the sensor with the sensor ID
   *
   * \param sensor_id The id of the sensor
   * \return The name of the sensor. Empty string on error
   */
  const std::string& getSensorName(uint sensor_id) const {
    if (sensor_id < elements_.size()) {
      return elements_[sensor_id].name;
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
    if (sensor_id < elements_.size()) {
      return elements_[sensor_id].pin_id;
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
    if (sensor_id < elements_.size()) {
      return elements_[sensor_id].unit;
    } else {
      return empty_;
    }
  }

 private:
  /// List of connected analog peripherials
  const std::array<AdcSensor, 6> elements_ = {
      {{32, "tds", 1.0, "mg/L"},       // Total dissolved solids
       {33, "ec", 1.0, "mS/cm"},       // Electrical conductivity
       {34, "acidity", 1.0, "pH"},     // Acidity
       {35, "turbidity", 1.0, "NTU"},  // Clarity of the water
       {36, "vn", 1.0, ""},            // Some other parameter
       {39, "vp", 1.0, ""}}};          // Some other parameter

  /// Returned when name of invalid sensor is requested
  const std::string empty_ = "";
};

}  // namespace bernd_box

#endif