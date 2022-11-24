#ifndef MINIMAL_BUILD
#include "as_ec_meter.h"

#include "peripheral/peripheral_factory.h"

namespace inamata {
namespace peripheral {
namespace peripherals {
namespace as_ec_meter {

AsEcMeterI2C::AsEcMeterI2C(const JsonObjectConst& parameters)
    : I2CAbstractPeripheral(parameters), Ezo_board(0) {
  // If the base class constructor failed, abort the constructor
  if (!isValid()) {
    return;
  }

  // Get and check the data point type for EC readings
  data_point_type_ = utils::UUID(parameters[data_point_type_key_]);
  if (!data_point_type_.isValid()) {
    setInvalid(data_point_type_key_error_);
    return;
  }
  // Get the probe type and check if the string is a float
  JsonVariantConst probe_type = parameters[probe_type_key_];
  if (!probe_type.is<const char*>()) {
    setInvalid(probe_type_key_error_);
    return;
  }
  if (atof(probe_type.as<const char*>()) == 0) {
    setInvalid(probe_type_cast_error_);
    return;
  }

  JsonVariantConst i2c_address_param = parameters[i2c_address_key_];
  if (!i2c_address_param.is<uint8_t>()) {
    setInvalid(i2c_address_key_error_);
    return;
  }
  // Save the I2C address to the Ezo_board's class i2c_address variable
  i2c_address = i2c_address_param.as<uint8_t>();

  // Do a preliminary check to see if the device is connected to the bus
  if (!isDeviceConnected(i2c_address)) {
    setInvalid(missingI2CDeviceError(i2c_address));
    return;
  }

  // Set the correct I2C interface for the Ezo_board driver class
  wire = getWire();

  // Configure probe type (range from K0.010 to K10.000)
  send_cmd((String(probe_type_code_) + probe_type.as<const char*>()).c_str());

  // Set to sleep mode
  send_cmd(String(sleep_code_).c_str());
}

const String& AsEcMeterI2C::getType() const { return type(); }

const String& AsEcMeterI2C::type() {
  static const String name{"AsEcMeterI2C"};
  return name;
}

capabilities::Calibrate::Result AsEcMeterI2C::startCalibration(
    const JsonObjectConst& parameters) {
  // Get the calibration command to be performed
  JsonVariantConst calibrate_command = parameters[calibrate_command_key_];
  if (!calibrate_command.is<const char*>()) {
    return {.wait = {},
            .error = ErrorResult(type(), calibrate_command_key_error_)};
  }

  // Each calibration process sends a code to the Ezo board ("Cal,dry",
  // "Cal,high,80000"), sets the type of calibration and the start time. It
  // then returns the expected time to complete the calibration process.
  // https://atlas-scientific.com/files/EC_EZO_Datasheet.pdf
  const String command = calibrate_command.as<String>();
  capabilities::Calibrate::Result result;
  if (command == calibrate_clear_command_) {
    result = startClearCalibration(parameters);
  } else if (command == calibrate_dry_command_) {
    result = startDryCalibration(parameters);
  } else if (command == calibrate_single_command_) {
    result = startSingleCalibration(parameters);
  } else if (command == calibrate_double_low_command_) {
    result = startDoubleLowCalibration(parameters);
  } else if (command == calibrate_double_high_command_) {
    result = startDoubleHighCalibration(parameters);
  } else {
    // No valid calibration command received
    calibration_state_ = CalibrationState::kNone;
    return {.wait = {}, .error = ErrorResult(type(), unknown_command_error_)};
  }
  // Perform timing setup for running the calibration, if the setup succeeded
  if (!result.error.isError()) {
    calibration_duration_ = result.wait;
    calibration_start_ = std::chrono::steady_clock::now();
  }
  return result;
}

capabilities::Calibrate::Result AsEcMeterI2C::handleCalibration() {
  auto elapsed_time = std::chrono::steady_clock::now() - calibration_start_;
  // Check waited long enough. Return the time remaining if not
  if (elapsed_time < calibration_duration_) {
    return {.wait = calibration_duration_ - elapsed_time};
  }
  // Check transitions
  capabilities::Calibrate::Result result;
  if (calibration_state_ == CalibrationState::kDryStabilize) {
    result = handleDryStabilizeCalibration();
  } else if (calibration_state_ == CalibrationState::kDrySet) {
    result = handleDrySetCalibration();
  } else if (calibration_state_ == CalibrationState::kSingleStabilize) {
    result = handleSingleStabilizeCalibration();
  } else if (calibration_state_ == CalibrationState::kSingleSet) {
    result = handleSingleSetCalibration();
  } else if (calibration_state_ == CalibrationState::kCheck) {
    result = handleCheckCalibration();
  } else if (calibration_state_ == CalibrationState::kDoubleLowStabilize) {
    result = handleDoubleLowStabilize();
  } else if (calibration_state_ == CalibrationState::kDoubleLowSet) {
    result = handleDoubleLowSet();
  } else if (calibration_state_ == CalibrationState::kDoubleHighStabilize) {
    result = handleDoubleHighStabilize();
  } else if (calibration_state_ == CalibrationState::kDoubleHighSet) {
    result = handleDoubleHighSet();
  } else if (calibration_state_ == CalibrationState::kClear) {
    // Finish clearing of calibration data
    calibration_state_ = CalibrationState::kNone;
    calibration_duration_ = std::chrono::milliseconds(0);

  } else {
    return {.wait = {},
            .error = ErrorResult(type(), invalid_transition_error_)};
  }
  if (!result.error.isError()) {
    calibration_duration_ = result.wait;
    calibration_start_ = std::chrono::steady_clock::now();
  }
  return result;
}

capabilities::StartMeasurement::Result AsEcMeterI2C::startMeasurement(
    const JsonVariantConst& parameters) {
  // Check if temperature compensation is enabled
  JsonVariantConst temperature_c = parameters[temperature_c_key_];
  if (temperature_c.is<float>()) {
    temperature_c_ = temperature_c;
  } else if (temperature_c.isNull()) {
    temperature_c_ = NAN;
  } else {
    return {.wait = {}, .error = ErrorResult(type(), temperature_c_key_error_)};
  }

  // Start reading type depending on whether temperature compoensation is set
  if (std::isnan(temperature_c_)) {
    send_read_cmd();
  } else {
    send_read_with_temp_comp(temperature_c_);
  }

  // Invalidate the last reading
  last_reading_ = NAN;

  return {.wait = reading_duration_};
}

capabilities::StartMeasurement::Result AsEcMeterI2C::handleMeasurement() {
  // Receive reading values, check if errors occured, check if measurement has
  // stabilized. Repeat if not stable.
  Ezo_board::errors error = receive_read_cmd();
  if (error == Ezo_board::errors::SUCCESS) {
    last_reading_ = reading;
    if (isReadingStable()) {
      return {.wait = {}};
    } else {
      // Start reading type depending if temperature compoensation is set
      if (std::isnan(temperature_c_)) {
        send_read_cmd();
      } else {
        send_read_with_temp_comp(temperature_c_);
      }
      return {.wait = reading_duration_};
    }
  } else if (error == Ezo_board::errors::NOT_READY) {
    // Be conservative. Wait another complete reading cycle
    return {.wait = reading_duration_};
  } else if (error == Ezo_board::errors::NO_DATA) {
    return {.wait = {}, ErrorResult(type(), F("No data"))};
  } else if (error == Ezo_board::errors::NOT_READ_CMD) {
    return {.wait = {}, ErrorResult(type(), F("Not started"))};
  } else {
    return {.wait = {}, ErrorResult(type(), F("Unknown error"))};
  }
}

capabilities::GetValues::Result AsEcMeterI2C::getValues() {
  // Use EC reading of last reading. Use startMeasurement capability. Invalidate
  // reading after returning it.
  if (!std::isnan(last_reading_)) {
    capabilities::GetValues::Result result = {
        .values = {utils::ValueUnit{.value = reading,
                                    .data_point_type = data_point_type_}}};
    last_reading_ = NAN;
    return result;
  } else {
    return {.values = {}, .error = ErrorResult(type(), get_values_error_)};
  }
}

capabilities::Calibrate::Result AsEcMeterI2C::startClearCalibration(
    const JsonObjectConst& parameters) {
  send_cmd(String(calibrate_clear_code_).c_str());
  calibration_state_ = CalibrationState::kClear;
  return {.wait = std::chrono::milliseconds(300)};
}

capabilities::Calibrate::Result AsEcMeterI2C::startDryCalibration(
    const JsonObjectConst& parameters) {
  if (calibration_state_ != CalibrationState::kNone) {
    calibration_state_ = CalibrationState::kNone;
    return {.wait = {},
            .error = ErrorResult(type(), invalid_transition_error_)};
  }
  // Request read to stabilize readings
  calibration_state_ = CalibrationState::kDryStabilize;
  last_reading_ = NAN;
  send_read_cmd();
  return {.wait = reading_duration_};
}

capabilities::Calibrate::Result AsEcMeterI2C::startSingleCalibration(
    const JsonObjectConst& parameters) {
  // Start the single point calibration process for the given value
  if (calibration_state_ != CalibrationState::kDryDone) {
    calibration_state_ = CalibrationState::kNone;
    return {.wait = {},
            .error = ErrorResult(type(), invalid_transition_error_)};
  }
  JsonVariantConst calibrate_value = parameters[calibrate_value_key_];
  if (!calibrate_value.is<int>()) {
    return {.wait = {},
            .error = ErrorResult(type(), calibrate_value_key_error_)};
  }
  calibrate_value_ = calibrate_value;
  JsonVariantConst temperature_c = parameters[temperature_c_key_];
  if (!temperature_c.is<int>()) {
    return {.wait = {}, .error = ErrorResult(type(), temperature_c_key_error_)};
  }
  temperature_c_ = temperature_c;

  // Request readings to stabilize readings
  calibration_state_ = CalibrationState::kSingleStabilize;
  last_reading_ = NAN;
  send_read_with_temp_comp(temperature_c_);
  return {.wait = reading_duration_};
}

capabilities::Calibrate::Result AsEcMeterI2C::startDoubleLowCalibration(
    const JsonObjectConst& parameters) {
  if (calibration_state_ != CalibrationState::kDryDone) {
    calibration_state_ = CalibrationState::kNone;
    return {.wait = {},
            .error = ErrorResult(type(), invalid_transition_error_)};
  }
  JsonVariantConst calibrate_value = parameters[calibrate_value_key_];
  if (!calibrate_value.is<int>()) {
    return {.wait = {},
            .error = ErrorResult(type(), calibrate_value_key_error_)};
  }
  calibrate_value_ = calibrate_value;
  JsonVariantConst temperature_c = parameters[temperature_c_key_];
  if (!temperature_c.is<int>()) {
    return {.wait = {}, .error = ErrorResult(type(), temperature_c_key_error_)};
  }
  temperature_c_ = temperature_c;

  // Request readings to stabilize readings
  calibration_state_ = CalibrationState::kDoubleLowStabilize;
  last_reading_ = NAN;
  send_read_with_temp_comp(temperature_c_);
  return {.wait = reading_duration_};
}

capabilities::Calibrate::Result AsEcMeterI2C::startDoubleHighCalibration(
    const JsonObjectConst& parameters) {
  if (calibration_state_ != CalibrationState::kDoubleLowDone) {
    calibration_state_ = CalibrationState::kNone;
    return {.wait = {},
            .error = ErrorResult(type(), invalid_transition_error_)};
  }
  JsonVariantConst calibrate_value = parameters[calibrate_value_key_];
  if (!calibrate_value.is<int>()) {
    return {.wait = {},
            .error = ErrorResult(type(), calibrate_value_key_error_)};
  }
  calibrate_value_ = calibrate_value;
  JsonVariantConst temperature_c = parameters[temperature_c_key_];
  if (!temperature_c.is<int>()) {
    return {.wait = {}, .error = ErrorResult(type(), temperature_c_key_error_)};
  }
  temperature_c_ = temperature_c;

  // Request readings to stabilize readings
  calibration_state_ = CalibrationState::kDoubleHighStabilize;
  last_reading_ = NAN;
  send_read_with_temp_comp(temperature_c_);
  return {.wait = reading_duration_};
}

capabilities::Calibrate::Result AsEcMeterI2C::handleDryStabilizeCalibration() {
  errors error = receive_read_cmd();
  if (error == errors::SUCCESS) {
    if (isReadingStable()) {
      send_cmd(String(calibrate_dry_code_).c_str());
      calibration_state_ = CalibrationState::kDrySet;
      return {.wait = std::chrono::milliseconds(600)};
    } else {
      send_read_cmd();
      return {.wait = reading_duration_};
    }
  } else {
    calibration_state_ = CalibrationState::kNone;
    return {.wait = {}, .error = ErrorResult(type(), receive_error_)};
  }
}

capabilities::Calibrate::Result AsEcMeterI2C::handleDrySetCalibration() {
  calibration_state_ = CalibrationState::kDryDone;
  return {.wait = {}};
}

capabilities::Calibrate::Result
AsEcMeterI2C::handleSingleStabilizeCalibration() {
  errors error = receive_read_cmd();
  if (error == errors::SUCCESS) {
    if (isReadingStable()) {
      send_cmd((String(calibrate_single_code_) + calibrate_value_).c_str());
      calibration_state_ = CalibrationState::kSingleSet;
      return {.wait = std::chrono::milliseconds(600)};
    } else {
      send_read_with_temp_comp(temperature_c_);
      return {.wait = reading_duration_};
    }
  } else {
    calibration_state_ = CalibrationState::kNone;
    return {.wait = {}, .error = ErrorResult(type(), receive_error_)};
  }
}

capabilities::Calibrate::Result AsEcMeterI2C::handleSingleSetCalibration() {
  calibration_state_ = CalibrationState::kCheck;
  send_cmd(String(calibrate_check_code_).c_str());
  return {.wait = std::chrono::milliseconds(300)};
}

capabilities::Calibrate::Result AsEcMeterI2C::handleCheckCalibration() {
  char sensor_data[this->bufferlen];
  errors error = receive_cmd(sensor_data, bufferlen);
  // If a response was received and it did not return the "not calibrated"
  // status, the calibration was successful
  if (error == errors::SUCCESS && String(not_calibrated_code_) != sensor_data) {
    calibration_state_ = CalibrationState::kNone;
    return {.wait = {}};
  } else {
    calibration_state_ = CalibrationState::kNone;
    return {.wait = {}, .error = ErrorResult(type(), not_calibrated_error_)};
  }
}

capabilities::Calibrate::Result AsEcMeterI2C::handleDoubleLowStabilize() {
  errors error = receive_read_cmd();
  if (error == errors::SUCCESS) {
    if (isReadingStable()) {
      send_cmd((String(calibrate_double_low_code_) + calibrate_value_).c_str());
      calibration_state_ = CalibrationState::kDoubleLowSet;
      return {.wait = std::chrono::milliseconds(600)};
    } else {
      send_read_with_temp_comp(temperature_c_);
      return {.wait = reading_duration_};
    }
  } else {
    calibration_state_ = CalibrationState::kNone;
    return {.wait = {}, .error = ErrorResult(type(), receive_error_)};
  }
}
capabilities::Calibrate::Result AsEcMeterI2C::handleDoubleLowSet() {
  calibration_state_ = CalibrationState::kDoubleLowDone;
  return {.wait = {}};
}

capabilities::Calibrate::Result AsEcMeterI2C::handleDoubleHighStabilize() {
  errors error = receive_read_cmd();
  if (error == errors::SUCCESS) {
    if (isReadingStable()) {
      send_cmd(
          (String(calibrate_double_high_code_) + calibrate_value_).c_str());
      calibration_state_ = CalibrationState::kDoubleHighSet;
      return {.wait = std::chrono::milliseconds(600)};
    } else {
      send_read_with_temp_comp(temperature_c_);
      return {.wait = reading_duration_};
    }
  } else {
    calibration_state_ = CalibrationState::kNone;
    return {.wait = {}, .error = ErrorResult(type(), receive_error_)};
  }
}

capabilities::Calibrate::Result AsEcMeterI2C::handleDoubleHighSet() {
  calibration_state_ = CalibrationState::kCheck;
  send_cmd(String(calibrate_check_code_).c_str());
  return {.wait = std::chrono::milliseconds(300)};
}

bool AsEcMeterI2C::isReadingStable() const {
  if (std::isnan(last_reading_)) {
    return false;
  }
  if (fabsf(last_reading_ - reading) / reading < stabalized_threshold_) {
    return true;
  } else {
    return false;
  }
}

std::shared_ptr<Peripheral> AsEcMeterI2C::factory(
    const ServiceGetters& services, const JsonObjectConst& parameters) {
  return std::make_shared<AsEcMeterI2C>(parameters);
}

bool AsEcMeterI2C::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

bool AsEcMeterI2C::capability_get_values_ =
    capabilities::GetValues::registerType(type());

bool AsEcMeterI2C::capability_calibrate_ =
    capabilities::Calibrate::registerType(type());

bool AsEcMeterI2C::capability_start_measurement_ =
    capabilities::StartMeasurement::registerType(type());

const __FlashStringHelper* AsEcMeterI2C::probe_type_key_ = FPSTR("probe_type");

const __FlashStringHelper* AsEcMeterI2C::probe_type_key_error_ =
    FPSTR("Missing property: probe_type (str)");

const __FlashStringHelper* AsEcMeterI2C::probe_type_cast_error_ =
    FPSTR("Can't cast probe type to float");

const __FlashStringHelper* AsEcMeterI2C::probe_type_code_ = FPSTR("K,");

const __FlashStringHelper* AsEcMeterI2C::temperature_c_key_ =
    FPSTR("temperature_c");

const __FlashStringHelper* AsEcMeterI2C::temperature_c_key_error_ =
    FPSTR("Wrong property: temperature_c (float)");

const __FlashStringHelper* AsEcMeterI2C::calibrate_command_key_ = FPSTR("command");

const __FlashStringHelper* AsEcMeterI2C::calibrate_command_key_error_ =
    FPSTR("Missing property: command (string)");

const __FlashStringHelper* AsEcMeterI2C::calibrate_value_key_ = FPSTR("value");

const __FlashStringHelper* AsEcMeterI2C::calibrate_value_key_error_ =
    FPSTR("Missing property: value (int)");

const __FlashStringHelper* AsEcMeterI2C::calibrate_dry_command_ = FPSTR("dry");

const __FlashStringHelper* AsEcMeterI2C::calibrate_dry_code_ = FPSTR("Cal,dry");

const __FlashStringHelper* AsEcMeterI2C::calibrate_single_command_ =
    FPSTR("single");

const __FlashStringHelper* AsEcMeterI2C::calibrate_single_code_ = FPSTR("Cal,");

const __FlashStringHelper* AsEcMeterI2C::calibrate_double_low_command_ =
    FPSTR("double_low");

const __FlashStringHelper* AsEcMeterI2C::calibrate_double_low_code_ =
    FPSTR("Cal,low,");

const __FlashStringHelper* AsEcMeterI2C::calibrate_double_high_command_ =
    FPSTR("double_high");

const __FlashStringHelper* AsEcMeterI2C::calibrate_double_high_code_ =
    FPSTR("Cal,high,");

const __FlashStringHelper* AsEcMeterI2C::calibrate_clear_command_ = FPSTR("clear");

const __FlashStringHelper* AsEcMeterI2C::calibrate_clear_code_ = FPSTR("Cal,clear");

const __FlashStringHelper* AsEcMeterI2C::calibrate_check_code_ = FPSTR("Cal,?");

const __FlashStringHelper* AsEcMeterI2C::not_calibrated_code_ = FPSTR("?Cal,0");

const __FlashStringHelper* AsEcMeterI2C::sleep_code_ = FPSTR("Sleep");

const __FlashStringHelper* AsEcMeterI2C::unknown_command_error_ =
    FPSTR("Unknown command");

const __FlashStringHelper* AsEcMeterI2C::invalid_transition_error_ =
    FPSTR("Invalid transition");

const __FlashStringHelper* AsEcMeterI2C::receive_error_ = FPSTR("Receive failed");

const __FlashStringHelper* AsEcMeterI2C::not_calibrated_error_ =
    FPSTR("Failed calibration");

}  // namespace as_ec_meter
}  // namespace peripherals
}  // namespace peripheral
}  // namespace inamata

#endif