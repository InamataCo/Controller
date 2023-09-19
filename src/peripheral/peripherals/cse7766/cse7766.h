#include "peripheral/capabilities/get_values.h"
#include "peripheral/capabilities/start_measurement.h"
#include "peripheral/peripherals/uart/uart_abstract_peripheral.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace cse7766 {


/**
 * Driver for CSE7766, an electrical energy measurement chip 
 * 
 * UART baud rate is 4800 bps with an 8E1 encoding but 8N1 also works. Reads a
 * data frame via the startMeasurement interface and values from the buffered
 * data frame can be read via the getValues interface.
 */
class CSE7766 : public uart::UARTAbstractPeripheral,
                public capabilities::GetValues,
                public capabilities::StartMeasurement {
 public:
  CSE7766(const JsonObjectConst& parameters);
  virtual ~CSE7766() = default;

  // Type registration in the peripheral factory
  const String& getType() const final;
  static const String& type();

  /**
   * Reset state and start reading data frame from CSE7766
   * 
   * @param parameters No parameters expected
   * @return Error, wait or ready depending on read frame
   */
  capabilities::StartMeasurement::Result startMeasurement(
      const JsonVariantConst& parameters) final;

  /**
   * Reads data frame CSE7766 and handle reset or timeout
   * 
   * - Attempt to read full and valid data frame from CSE7766 via UART/serial
   * - Reset state after having read the values and start new timeout
   * - Return timeout error if values not ready or read due to timeout
   * - Return wait on no data or too many invalid bytes
   *
   * \return A vector with all read data points and their type
   */
  capabilities::StartMeasurement::Result handleMeasurement() final;

  /**
   * Read values measured by CSE7766 from sent data frame
   * 
   * @return Measured volts, amps and watts or error
   */
  capabilities::GetValues::Result getValues() final;

 private:
  static std::shared_ptr<Peripheral> factory(const ServiceGetters& services,
                                             const JsonObjectConst& parameters);

  /**
   * Read data frame until valid frame or fault occurs
   * 
   * @return Blank on success, wait if fault occurs
   */
  capabilities::StartMeasurement::Result readFrame();

  /**
   * Reset frame buffer, error counter and timeout
   */
  void resetState();

  /**
   * Parse a 24-bit MSB/big-endian uint
   *
   * @param first_byte Pointer to first byte of 24-bit uint
   * @return Parsed uint stored in a native 32-bit uint
   */
  static uint32_t parse24bit(uint8_t* first_byte);

  /**
   * Parse a 16-bit MSB/big-endian uint
   *
   * @param first_byte Pointer to first byte of 16-bit uint
   * @return Parsed uint stored in a native 32-bit uint
   */
  static uint32_t parse16bit(uint8_t* first_byte);

  static bool registered_;
  static bool capability_get_values_;
  static bool capability_start_measurement_;

  /// Buffer for a data frame (frame is 24 bytes)
  uint8_t in_data_[24];
  /// Index of the data frame buffer
  uint8_t in_data_i_;
  /// Number of invalid bytes read
  uint32_t invalid_byte_count_;
  /// Whether values from frame buffer were read
  bool got_values_ = false;
  /// When reading the frame buffer started (for timeout)
  std::chrono::steady_clock::time_point measurement_start_;
  /// Max duration to fill frame buffer
  static constexpr std::chrono::seconds measurement_timeout_{10};
  /// Time to wait after too many read errors or no data available
  static constexpr std::chrono::milliseconds no_data_wait_{500};

  /// Bitmask if power was measured 
  static constexpr uint8_t adj_power_mask = 1 << 4;
  /// Bitmask if current was measured
  static constexpr uint8_t adj_current_mask = 1 << 5;
  /// Bitmask if voltage was measured
  static constexpr uint8_t adj_voltage_mask = 1 << 6;

  /// Byte offset (position) in frame for voltage coefficient
  static constexpr uint8_t voltage_coef_offset = 2;
  /// Byte offset (position) in frame for voltage cycle
  static constexpr uint8_t voltage_cycle_offset = 5;
  /// Byte offset (position) in frame for current coefficient
  static constexpr uint8_t current_coef_offset = 8;
  /// Byte offset (position) in frame for current cycle
  static constexpr uint8_t current_cycle_offset = 11;
  /// Byte offset (position) in frame for power coefficient
  static constexpr uint8_t power_coef_offset = 14;
  /// Byte offset (position) in frame for power cycle
  static constexpr uint8_t power_cycles_offset = 17;
  /// Byte offset (position) in frame for 'adj' flags
  static constexpr uint8_t adj_offset = 20;
  /// Byte offset (position) in frame for frame checksum
  static constexpr uint8_t checksum_offset = 23;

  /// Bitmask for header 1 byte if errors occured
  static constexpr uint8_t header_error_mask = 0xF0;
  /// Power Out-of-Range error - No electrical consumer / no current
  static constexpr uint8_t power_oor_value = header_error_mask | 1 << 1;
  /// Current Out-of-Range error - No electrical consumer / no current
  static constexpr uint8_t current_oor_value = header_error_mask | 1 << 2;
  /// Voltage Out-of-Range error - No electrical consumer / no current
  static constexpr uint8_t voltage_oor_value = header_error_mask | 1 << 3;

  /// ID of voltage data point type
  utils::UUID voltage_data_point_type_{nullptr};
  /// Key in parameters dict for the ID of voltage data point type
  static const __FlashStringHelper* voltage_data_point_type_key_;

  /// ID of current data point type
  utils::UUID current_data_point_type_{nullptr};
  /// Key in parameters dict for the ID of current data point type
  static const __FlashStringHelper* current_data_point_type_key_;

  /// ID of power data point type
  utils::UUID power_data_point_type_{nullptr};
  /// Key in parameters dict for the ID of power data point type
  static const __FlashStringHelper* power_data_point_type_key_;
};

}  // namespace cse7766
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata
