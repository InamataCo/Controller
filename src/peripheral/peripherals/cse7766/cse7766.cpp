#include "cse7766.h"

#include "peripheral/peripheral_factory.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace cse7766 {

CSE7766::CSE7766(const JsonObjectConst& parameters)
    : UARTAbstractPeripheral(parameters) {
  // If the base class constructor failed, abort the constructor
  if (!isValid()) {
    return;
  }

  // Get and check the data point type for voltage readings
  voltage_data_point_type_ =
      utils::UUID(parameters[voltage_data_point_type_key_]);
  if (!voltage_data_point_type_.isValid()) {
    setInvalid(ErrorStore::genMissingProperty(voltage_data_point_type_key_,
                                              ErrorStore::KeyType::kUUID));
    return;
  }

  // Get and check the data point type for current readings
  current_data_point_type_ =
      utils::UUID(parameters[current_data_point_type_key_]);
  if (!current_data_point_type_.isValid()) {
    setInvalid(ErrorStore::genMissingProperty(current_data_point_type_key_,
                                              ErrorStore::KeyType::kUUID));
    return;
  }

  // Get and check the data point type for power readings
  power_data_point_type_ = utils::UUID(parameters[power_data_point_type_key_]);
  if (!power_data_point_type_.isValid()) {
    setInvalid(ErrorStore::genMissingProperty(power_data_point_type_key_,
                                              ErrorStore::KeyType::kUUID));
    return;
  }
}

const String& CSE7766::getType() const { return type(); }

const String& CSE7766::type() {
  static const String name{"CSE7766"};
  return name;
}

capabilities::StartMeasurement::Result CSE7766::startMeasurement(
    const JsonVariantConst& parameters) {
  resetState();
  return readFrame();
}

capabilities::StartMeasurement::Result CSE7766::handleMeasurement() {
  if (got_values_) {
    resetState();
  }
  if (std::chrono::steady_clock::now() - measurement_start_ >
      measurement_timeout_) {
    return capabilities::StartMeasurement::Result{
        .error = ErrorResult(type(), F("Timeout"))};
  }
  return readFrame();
}

capabilities::GetValues::Result CSE7766::getValues() {
  if (in_data_[0] == 0xAA) {
    return capabilities::GetValues::Result{
        .error = ErrorResult(type(), F("Not calibrated"))};
  }

  capabilities::GetValues::Result result;
  const uint8_t adj = in_data_[adj_offset];

  if (adj & adj_voltage_mask) {
    const double voltage_coef = parse24bit(in_data_ + voltage_coef_offset);
    const double voltage_cycle = parse24bit(in_data_ + voltage_cycle_offset);
    const float voltage = voltage_coef / voltage_cycle;
    result.values.push_back(utils::ValueUnit{
        .value = voltage, .data_point_type = voltage_data_point_type_});
  }
  bool is_power_valid = false;
  float power = 0.0;
  if (adj & adj_power_mask) {
    // Ensure abnormal header and power out-of-range bit are not set.
    // Otherwise set power as off
    if ((in_data_[0] & power_oor_value) != power_oor_value) {
      is_power_valid = true;
      const double power_coef = parse24bit(in_data_ + power_coef_offset);
      const double power_cycle = parse24bit(in_data_ + power_cycles_offset);
      power = power_coef / power_cycle;
    }
  }
  // If power is not valid, current is also
  float current = 0.0;
  if (adj & adj_current_mask) {
    if (is_power_valid) {
      const double current_coef = parse24bit(in_data_ + current_coef_offset);
      const double current_cycle = parse24bit(in_data_ + current_cycle_offset);
      current = current_coef / current_cycle;
    }
  }
  // Ensure that if current or power are measured, always send both
  if (adj & adj_current_mask || adj & adj_power_mask) {
    result.values.push_back(utils::ValueUnit{
        .value = power, .data_point_type = power_data_point_type_});
    result.values.push_back(utils::ValueUnit{
        .value = current, .data_point_type = current_data_point_type_});
  }

  // Mark frame as being read and allow frame to be cleared for next poll
  got_values_ = true;

  return result;
}

capabilities::StartMeasurement::Result CSE7766::readFrame() {
  while (true) {
    // Check if a valid frame has been read (once frame is full).
    if (in_data_i_ >= sizeof(in_data_)) {
      // Sum all bytes (with overflow) excluding header and checksum bytes
      uint8_t checksum = 0;
      for (uint8_t i = 2; i < checksum_offset; i++) {
        checksum += in_data_[i];
      }
      // On invalid checksum, read new frame
      if (checksum != in_data_[checksum_offset]) {
        in_data_i_ = 0;
        invalid_byte_count_ += sizeof(in_data_);
        continue;
      } else {
        // Frame read successfully, can now get values from frame
        return capabilities::StartMeasurement::Result();
      }
    }

    // Read bytes while available (not -1), wait if no valid data available
    int byte_read = uart_adapter_->getSerial()->read();
    if (byte_read == -1) {
      return capabilities::StartMeasurement::Result{.wait = no_data_wait_};
    }
    // If more than two frame lengths of invalid bytes were read, then
    // clear input buffer (at most 512 bytes) and wait for new attempt
    if (invalid_byte_count_ &&
        invalid_byte_count_ % (sizeof(in_data_) * 2) == 0) {
      HardwareSerial* serial = uart_adapter_->getSerial();
      uint8_t buffer[64];
      for (int i = 0; i < 8; i++) {
        // Will block for 1s due to HardwareSerial timeout
        size_t byte_count = serial->readBytes(buffer, sizeof(buffer));
        if (byte_count != sizeof(buffer)) {
          break;
        }
      }
      invalid_byte_count_++;
      return capabilities::StartMeasurement::Result{.wait = no_data_wait_};
    }

    // If first byte is invalid, read next byte and check it
    if (in_data_i_ == 0 && byte_read != 0x55 && byte_read < 0xF0 &&
        byte_read != 0xAA) {
      invalid_byte_count_++;
      continue;
    }
    // If the second byte is invalid, restart reading frame
    if (in_data_i_ == 1 && byte_read != 0x5A) {
      in_data_i_ = 0;
      invalid_byte_count_ += 2;
      continue;
    }
    // Save byte at index and increment index
    in_data_[in_data_i_] = byte_read;
    in_data_i_++;
  }
}

void CSE7766::resetState() {
  // Set after reading frame and getting values
  got_values_ = false;
  // Clear frame data and errors
  memset(in_data_, 0, sizeof(in_data_));
  in_data_i_ = 0;
  invalid_byte_count_ = 0;
  // Start time of reading a new frame
  measurement_start_ = std::chrono::steady_clock::now();
}

uint32_t CSE7766::parse24bit(uint8_t* first_byte) {
  return uint32_t(*first_byte) << 16 | uint32_t(*(first_byte + 1)) << 8 |
         uint32_t(*(first_byte + 2));
}

uint32_t CSE7766::parse16bit(uint8_t* first_byte) {
  return uint32_t(*first_byte) << 8 | uint32_t(*(first_byte + 1));
}

std::shared_ptr<Peripheral> CSE7766::factory(
    const ServiceGetters& services, const JsonObjectConst& parameters) {
  return std::make_shared<CSE7766>(parameters);
}

bool CSE7766::registered_ = PeripheralFactory::registerFactory(type(), factory);

bool CSE7766::capability_get_values_ =
    capabilities::GetValues::registerType(type());

bool CSE7766::capability_start_measurement_ =
    capabilities::StartMeasurement::registerType(type());

const __FlashStringHelper* CSE7766::voltage_data_point_type_key_ =
    FPSTR("voltage_data_point_type");
const __FlashStringHelper* CSE7766::current_data_point_type_key_ =
    FPSTR("current_data_point_type");
const __FlashStringHelper* CSE7766::power_data_point_type_key_ =
    FPSTR("power_data_point_type");

}  // namespace cse7766
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata