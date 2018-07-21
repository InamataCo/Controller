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
  float read(uint element_id) {
    float value = NAN;

    // If the element ID is valid, read from that pin and scale appropriately
    if (element_id < elements_.size()) {
      value = analogRead(elements_[element_id].pin_id);
      value *= elements_[element_id].scaling_factor;
    }

    return value;
  }

  uint getSensorCount() const { return elements_.size(); }

 private:
  /// List of connected analog peripherials
  const std::array<AdcSensor, 6> elements_ = {
      {{32, "tds", 1.0, "mg/L"},    // Total dissolved solids
       {33, "ec", 1.0, "mS/cm"},    // Electrical conductivity
       {34, "acidity", 1.0, "pH"},  // Acidity
       {35, "turbidity", 1.0, ""},  // Clarity of the water
       {36, "vn", 1.0, ""},         // Some other parameter
       {39, "vp", 1.0, ""}}};       // Some other parameter
};

}  // namespace bernd_box

#endif