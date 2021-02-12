#pragma once

#include <ArduinoJson.h>
#include <Ezo_i2c.h>

#include "peripheral/capabilities/calibrate.h"
#include "peripheral/capabilities/get_values.h"
#include "peripheral/capabilities/start_measurement.h"
#include "peripheral/peripherals/i2c_adapter/i2c_abstract_peripheral.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace as_ec_meter {

/**
 * Peripheral interface for the Atlas Scientific EZO EC Meter over I2C
 */
class AsEcMeterI2C : public peripherals::i2c_adapter::I2CAbstractPeripheral,
                     public capabilities::GetValues,
                     public capabilities::StartMeasurement,
                     public capabilities::Calibrate,
                     private Ezo_board {
 public:
  AsEcMeterI2C(const JsonObjectConst& parameters);
  virtual ~AsEcMeterI2C() = default;

  // Type registration in the peripheral factory
  const String& getType() const final;
  static const String& type();

  /**
   * Perform a calibration step on the peripheral
   *
   * \param parameters The peripheral specific calibration command
   * \return The time to wait to call handleCalibration(), if an error occured,
   *     or if the calibration step is complete
   */
  capabilities::Calibrate::Result startCalibration(
      const JsonObjectConst& parameters) final;
  
  /**
   * Handle a calibration step
   * 
   * \return The time to wait to call handleCalibration() again, if an error
   *    occured or if the calibration step is complete
   */
  capabilities::Calibrate::Result handleCalibration();

  /**
   * Start an EC measurement
   * 
   * \param parameters Optionally accepts temperature_c as the EC meter's water
   *                   temperature
   * \return The time until the result is ready to be read (~1s)
   */
  capabilities::StartMeasurement::Result startMeasurement(
      const JsonVariantConst& parameters) final;

  /**
   * Check the measurement state
   * 
   * If the measurement is not stable yet, it returns a wait time until the
   * next reading has been taken.
   *
   * \return The time to call measurementState() again, if an error occured or
   *    if the measurement is complete
   */
  capabilities::StartMeasurement::Result measurementState() final;

  /**
   * Reads the EC value
   *
   * \return A vector with the latest EC value
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

  static std::shared_ptr<Peripheral> factory(const JsonObjectConst& parameters);
  static bool registered_;
  static bool capability_get_values_;

  utils::UUID data_point_type_{nullptr};

  static const __FlashStringHelper* probe_type_key_;
  static const __FlashStringHelper* probe_type_key_error_;
  static const __FlashStringHelper* probe_type_code_;

  float stabalized_threshold_{0.01};

  // Reading
  std::chrono::steady_clock::time_point reading_start_;
  const std::chrono::seconds reading_duration_{1};
  float last_reading_;

  // Calibration
  /// Calibration temperature
  float temperature_c_;
  static const __FlashStringHelper* temperature_c_key_;
  static const __FlashStringHelper* temperature_c_key_error_;

  /**
   * State transitions:
   * kNone --> kDryStabilize --> kDrySet --> %kDryDone --> kSingleStabilize
   *       --> kSingleSet --> kCheck --> kNone
   * kNone --> kDryStabilize --> kDrySet --> %kDryDone --> kDoubleLowStabilize
   *       --> kDoubleLowSet --> %kDoubleLowDone --> kDoubleHighStabilize
   *       --> kDoubleHighSet --> kCheck --> kNone
   * * --> kClear --> kNone
   * On error go to kNone.
   * The % sign signifies that a calibration step completed and requires user
   * input to commence the next calibration step.
   * The * sign signifies that this step can be started from any state
   */
  enum class CalibrationState {
    kNone,
    kDryStabilize,
    kDrySet,
    kDryDone,
    kSingleStabilize,
    kSingleSet,
    kDoubleLowStabilize,
    kDoubleLowSet,
    kDoubleLowDone,
    kDoubleHighStabilize,
    kDoubleHighSet,
    kCheck,
    kClear,
  } calibration_state_;
  std::chrono::steady_clock::time_point calibration_start_;
  std::chrono::nanoseconds calibration_duration_;
  
  float calibrate_value_;
  static const __FlashStringHelper* calibrate_command_key_;
  static const __FlashStringHelper* calibrate_command_key_error_;
  
  static const __FlashStringHelper* calibrate_value_key_;
  static const __FlashStringHelper* calibrate_value_key_error_;
  
  static const __FlashStringHelper* calibrate_dry_command_;
  static const __FlashStringHelper* calibrate_dry_code_;
  
  static const __FlashStringHelper* calibrate_single_command_;
  static const __FlashStringHelper* calibrate_single_code_;
  
  static const __FlashStringHelper* calibrate_double_low_command_;
  static const __FlashStringHelper* calibrate_double_low_code_;
  
  static const __FlashStringHelper* calibrate_double_high_command_;
  static const __FlashStringHelper* calibrate_double_high_code_;
  
  static const __FlashStringHelper* calibrate_clear_command_;
  static const __FlashStringHelper* calibrate_clear_code_;
  
  static const __FlashStringHelper* calibrate_check_code_;
  static const __FlashStringHelper* not_calibrated_code_;
  static const __FlashStringHelper* sleep_code_;
  
  static const __FlashStringHelper* unknown_command_error_;
  static const __FlashStringHelper* invalid_transition_error_;
  static const __FlashStringHelper* receive_error_;
  static const __FlashStringHelper* not_calibrated_error_;
};

}  // namespace as_ec_meter
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
