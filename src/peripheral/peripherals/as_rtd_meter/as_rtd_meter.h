#pragma once

#include <ArduinoJson.h>
#include <Ezo_i2c.h>

#include "peripheral/capabilities/get_values.h"
#include "peripheral/capabilities/start_measurement.h"
#include "peripheral/peripherals/i2c_adapter/i2c_abstract_peripheral.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace as_rtd_meter {

class AsRtdMeterI2C : public peripherals::i2c_adapter::I2CAbstractPeripheral,
                      public capabilities::GetValues,
                      public capabilities::StartMeasurement,
                      private Ezo_board {
 public:
  AsRtdMeterI2C(const JsonVariantConst& parameters);
  virtual ~AsRtdMeterI2C();

  // Type registration in the peripheral factory
  const String& getType() const final;
  static const String& type();

  /**
   * Start an temperature measurement
   *
   * \param parameters
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
   * \return The time wait, if an error occured or if the measurement completed
   */
  capabilities::StartMeasurement::Result handleMeasurement() final;

  /**
   * Reads the temperature value generated from the startMeasurement capability
   *
   * Invalidates the reading after returning it. Repeat startMeasurement for
   * new readings
   *
   * \return A vector with the latest temperature value or if an error occured
   */
  capabilities::GetValues::Result getValues() final;

 private:
  utils::UUID data_point_type_{nullptr};

  // Reading
  const std::chrono::milliseconds reading_duration_{600};
  float last_reading_ = NAN;

  static const __FlashStringHelper* sleep_code_;
};

}  // namespace as_rtd_meter
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box