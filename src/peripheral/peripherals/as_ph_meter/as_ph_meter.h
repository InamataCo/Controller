#pragma once

#include <ArduinoJson.h>
#include <Ezo_i2c.h>

#include "peripheral/capabilities/get_values.h"
#include "peripheral/capabilities/start_measurement.h"
#include "peripheral/peripherals/i2c/i2c_abstract_peripheral.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace as_ph_meter {

class AsPhMeterI2C : public peripherals::i2c::I2CAbstractPeripheral,
                     public capabilities::GetValues,
                     public capabilities::StartMeasurement,
                     private Ezo_board {
 public:
  AsPhMeterI2C(const JsonVariantConst& parameters);
  virtual ~AsPhMeterI2C() = default;

  // Type registration in the peripheral factory
  const String& getType() const final;
  static const String& type();

  /**
   * Start a pH measurement
   *
   * \param parameters
   * \return The time until the result is ready to be read (~900ms)
   */
  capabilities::StartMeasurement::Result startMeasurement(
      const JsonVariantConst& parameters) final;

  /**
   * Check the measurement state
   *
   * If the measurement is not stable yet, it returns a wait time until the
   * next reading has been taken.
   *
   * \return The time wait, if an error occured or if the measurement completed
   */
  capabilities::StartMeasurement::Result handleMeasurement() final;

  /**
   * Reads the pH value generated from the startMeasurement capability
   *
   * Invalidates the reading after returning it. Repeat startMeasurement for
   * new readings
   *
   * \return A vector with the latest temperature value or if an error occured
   */
  capabilities::GetValues::Result getValues() final;

 private:
  /**
   * Checks if the reading has stabilizied within a specified range
   *
   * Uses the reading member set by Ezo_board after calling receive_read_cmd()
   *
   * \return True if stable
   */
  bool isReadingStable() const;

  static std::shared_ptr<Peripheral> factory(const ServiceGetters& services,
                                             const JsonObjectConst& parameters);
  static bool registered_;
  static bool capability_get_values_;
  static bool capability_start_measurement_;

  utils::UUID data_point_type_{nullptr};

  float stabalized_threshold_{0.1};

  // Reading
  const std::chrono::milliseconds reading_duration_{900};
  float last_reading_ = NAN;

  // Temperature compensation
  float temperature_c_;
  static const __FlashStringHelper* temperature_c_key_;
  static const __FlashStringHelper* temperature_c_key_error_;

  static const __FlashStringHelper* sleep_code_;
};

}  // namespace as_rtd_meter
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata