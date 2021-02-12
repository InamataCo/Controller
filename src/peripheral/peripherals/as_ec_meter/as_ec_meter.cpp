#include "as_ec_meter.h"

namespace bernd_box {
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
  // Get the probe type
  JsonVariantConst probe_type = parameters[probe_type_key_];
  if (!probe_type.is<float>()) {
    setInvalid(probe_type_key_error_);
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
  send_cmd((String(probe_type_code_) + probe_type.as<float>()).c_str());

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
  if (calibrate_command.isNull() || !calibrate_command.is<char*>()) {
    return {.wait = {},
            .error = ErrorResult(type(), calibrate_command_key_error_)};
  }

  // Each calibration process sends a code to the Ezo board ("Cal,dry",
  // "Cal,high,80000"), sets the type of calibration and the start time. It
  // then returns the expected time to complete the calibration process.
  // https://atlas-scientific.com/files/EC_EZO_Datasheet.pdf
  const String command = calibrate_command.as<String>();
  if (command == calibrate_clear_command_) {
    // Clear the calibration settings on the Ezo_board
    send_cmd(String(calibrate_clear_code_).c_str());
    calibration_duration_ = std::chrono::milliseconds(300);
    calibration_state_ = CalibrationState::kClear;

  } else if (command == calibrate_dry_command_) {
    // Start the dry calibration process
    if (calibration_state_ != CalibrationState::kNone) {
      calibration_state_ = CalibrationState::kNone;
      return {.wait = {},
              .error = ErrorResult(type(), invalid_transition_error_)};
    }
    // Request readings to stabilize readings
    send_read_cmd();
    calibration_duration_ = reading_duration_;
    calibration_state_ = CalibrationState::kDryStabilize;

  } else if (command == calibrate_single_command_) {
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
      return {.wait = {},
              .error = ErrorResult(type(), temperature_c_key_error_)};
    }
    temperature_c_ = temperature_c;

    // Request readings to stabilize readings
    send_read_with_temp_comp(temperature_c_);
    calibration_duration_ = reading_duration_;
    calibration_state_ = CalibrationState::kSingleStabilize;

  } else if (command == calibrate_double_low_command_) {
    // Start the lower of the two point calibration process
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
      return {.wait = {},
              .error = ErrorResult(type(), temperature_c_key_error_)};
    }
    temperature_c_ = temperature_c;

    // Request readings to stabilize readings
    send_read_with_temp_comp(temperature_c_);
    calibration_duration_ = reading_duration_;
    calibration_state_ = CalibrationState::kDoubleLowStabilize;

  } else if (command == calibrate_double_high_command_) {
    // Start the upper of the two point calibration process
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
      return {.wait = {},
              .error = ErrorResult(type(), temperature_c_key_error_)};
    }
    temperature_c_ = temperature_c;

    // Request readings to stabilize readings
    send_read_with_temp_comp(temperature_c_);
    calibration_duration_ = reading_duration_;
    calibration_state_ = CalibrationState::kDoubleHighStabilize;

  } else {
    // No valid calibration command received
    calibration_state_ = CalibrationState::kNone;
    return {.wait = {}, .error = ErrorResult(type(), unknown_command_error_)};
  }
  calibration_start_ = std::chrono::steady_clock::now();
  return {.wait = calibration_duration_};
}

capabilities::Calibrate::Result AsEcMeterI2C::handleCalibration() {
  auto elapsed_time = std::chrono::steady_clock::now() - calibration_start_;
  // Check if the calibration had enough time to complete. Return the time
  // remaining and request to retry.
  if (elapsed_time < calibration_duration_) {
    return {.wait = calibration_duration_ - elapsed_time};
  }

  // Check transitions
  if (calibration_state_ == CalibrationState::kDryStabilize) {
    // Check if readings have stabilized and the dry point can be calibrated
    errors error = receive_read_cmd();
    if (error == errors::SUCCESS) {
      if (isReadingStable()) {
        send_cmd(String(calibrate_dry_code_).c_str());
        calibration_duration_ = std::chrono::milliseconds(600);
        calibration_state_ = CalibrationState::kDrySet;
      } else {
        send_read_cmd();
        calibration_duration_ = reading_duration_;
      }
    } else {
      calibration_state_ = CalibrationState::kNone;
      return {.wait = {}, .error = ErrorResult(type(), receive_error_)};
    }
  }

  if (calibration_state_ == CalibrationState::kDrySet) {
    // Finish dry calibration step. Wait for new calibration command
    calibration_state_ = CalibrationState::kDryDone;
    calibration_duration_ = std::chrono::milliseconds(0);

  } else if (calibration_state_ == CalibrationState::kSingleStabilize) {
    // Check if readings have stabilized and the single point can be calibrated
    errors error = receive_read_cmd();
    if (error == errors::SUCCESS) {
      if (isReadingStable()) {
        send_cmd((String(calibrate_single_code_) + calibrate_value_).c_str());
        calibration_duration_ = std::chrono::milliseconds(600);
        calibration_state_ = CalibrationState::kSingleSet;
      } else {
        send_read_with_temp_comp(temperature_c_);
        calibration_duration_ = reading_duration_;
      }
    } else {
      calibration_state_ = CalibrationState::kNone;
      return {.wait = {}, .error = ErrorResult(type(), receive_error_)};
    }

  } else if (calibration_state_ == CalibrationState::kSingleSet) {
    // Finish single calibration step. Check if succeeded
    calibration_state_ = CalibrationState::kCheck;
    send_cmd(String(calibrate_check_code_).c_str());
    calibration_duration_ = std::chrono::milliseconds(300);

  } else if (calibration_state_ == CalibrationState::kCheck) {
    // Finish calibration check. Go to initial state
    char sensor_data[this->bufferlen];
    errors error = receive_cmd(sensor_data, bufferlen);
    // If a response was received and it did not send the not calibrated
    // status, the calibration was successful
    if (error == errors::SUCCESS &&
        String(not_calibrated_code_) != sensor_data) {
      calibration_state_ = CalibrationState::kNone;
      calibration_duration_ = std::chrono::milliseconds(0);
    } else {
      calibration_state_ = CalibrationState::kNone;
      return {.wait = std::chrono::milliseconds(0),
              .error = ErrorResult(type(), not_calibrated_error_)};
    }

  } else if (calibration_state_ == CalibrationState::kDoubleLowStabilize) {
    // Check if readings have stabilized and the single point can be calibrated
    errors error = receive_read_cmd();
    if (error == errors::SUCCESS) {
      if (isReadingStable()) {
        send_cmd(
            (String(calibrate_double_low_code_) + calibrate_value_).c_str());
        calibration_duration_ = std::chrono::milliseconds(600);
        calibration_state_ = CalibrationState::kDoubleLowSet;
      } else {
        send_read_with_temp_comp(temperature_c_);
        calibration_duration_ = reading_duration_;
      }
    } else {
      calibration_state_ = CalibrationState::kNone;
      return {.wait = {}, .error = ErrorResult(type(), receive_error_)};
    }

  } else if (calibration_state_ == CalibrationState::kDoubleLowSet) {
    // Finish single calibration step. Wait for new calibration command
    calibration_state_ = CalibrationState::kDoubleLowDone;
    calibration_duration_ = std::chrono::milliseconds(0);

  } else if (calibration_state_ == CalibrationState::kDoubleHighStabilize) {
    // Check if readings have stabilized and the single point can be calibrated
    errors error = receive_read_cmd();
    if (error == errors::SUCCESS) {
      if (isReadingStable()) {
        send_cmd(
            (String(calibrate_double_high_code_) + calibrate_value_).c_str());
        calibration_duration_ = std::chrono::milliseconds(600);
        calibration_state_ = CalibrationState::kDoubleHighSet;
      } else {
        send_read_with_temp_comp(temperature_c_);
        calibration_duration_ = reading_duration_;
      }
    } else {
      calibration_state_ = CalibrationState::kNone;
      return {.wait = {}, .error = ErrorResult(type(), receive_error_)};
    }

  } else if (calibration_state_ == CalibrationState::kDoubleHighSet) {
    // Finish single calibration step. Check if succeeded
    calibration_state_ = CalibrationState::kCheck;
    send_cmd(String(calibrate_check_code_).c_str());
    calibration_duration_ = std::chrono::milliseconds(300);

  } else if (calibration_state_ == CalibrationState::kClear) {
    // Finish clearing of calibration data
    calibration_state_ = CalibrationState::kNone;
    calibration_duration_ = std::chrono::milliseconds(0);

  } else {
    return {.wait = std::chrono::milliseconds(0),
            .error = ErrorResult(type(), invalid_transition_error_)};
  }
  return {.wait = calibration_duration_};
}

capabilities::StartMeasurement::Result AsEcMeterI2C::startMeasurement(
    const JsonVariantConst& parameters) {
  // Invalidate the last reading
  last_reading_ = NAN;

  JsonVariantConst temperature_c = parameters[temperature_c_key_];
  if (temperature_c.is<float>()) {
    send_read_with_temp_comp(temperature_c);
  } else if (temperature_c.isNull()) {
    send_read_cmd();
  } else {
    return {.wait = {}, .error = ErrorResult(type(), temperature_c_key_error_)};
  }
  return {.wait = reading_duration_};
}

capabilities::StartMeasurement::Result AsEcMeterI2C::measurementState() {
  Ezo_board::errors error = receive_read_cmd();
  if (error == Ezo_board::errors::SUCCESS) {
    if (isReadingStable()) {
      last_reading_ = reading;
      return {.wait = {}};
    } else {
      last_reading_ = reading;
      return {.wait = reading_duration_};
    }
  } else if (error == Ezo_board::errors::NOT_READY) {
    auto elapsed_time = std::chrono::steady_clock::now() - reading_start_;
    return {.wait = reading_duration_ - elapsed_time};
  } else if (error == Ezo_board::errors::NO_DATA) {
    return {.wait = {}, ErrorResult(type(), F("No data"))};
  } else if (error == Ezo_board::errors::NOT_READ_CMD) {
    return {.wait = {}, ErrorResult(type(), F("Not started"))};
  } else {
    return {.wait = {}, ErrorResult(type(), F("Unknown error"))};
  }
}

capabilities::GetValues::Result AsEcMeterI2C::getValues() {
  // Request the EC reading. On success return the reading, else an error
  errors error = receive_read_cmd();
  if (error == errors::SUCCESS) {
    return {.values = {utils::ValueUnit{.value = reading,
                                        .data_point_type = data_point_type_}}};
  } else {
    return {.values = {}, .error = ErrorResult(type(), get_values_error_)};
  }
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
    const JsonObjectConst& parameters) {
  return std::make_shared<AsEcMeterI2C>(parameters);
}

bool AsEcMeterI2C::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

bool AsEcMeterI2C::capability_get_values_ =
    capabilities::GetValues::registerType(type());

const __FlashStringHelper* AsEcMeterI2C::probe_type_key_ = F("probe_type");

const __FlashStringHelper* AsEcMeterI2C::probe_type_key_error_ =
    F("Missing property: probe_type (float)");

const __FlashStringHelper* AsEcMeterI2C::probe_type_code_ = F("K,");

const __FlashStringHelper* AsEcMeterI2C::temperature_c_key_ =
    F("temperature_c");

const __FlashStringHelper* AsEcMeterI2C::temperature_c_key_error_ =
    F("Wrong property: temperature_c (float)");

const __FlashStringHelper* AsEcMeterI2C::calibrate_command_key_ = F("command");

const __FlashStringHelper* AsEcMeterI2C::calibrate_command_key_error_ =
    F("Missing property: command (string)");

const __FlashStringHelper* AsEcMeterI2C::calibrate_value_key_ = F("value");

const __FlashStringHelper* AsEcMeterI2C::calibrate_value_key_error_ =
    F("Missing property: value (int)");

const __FlashStringHelper* AsEcMeterI2C::calibrate_dry_command_ = F("dry");

const __FlashStringHelper* AsEcMeterI2C::calibrate_dry_code_ = F("Cal,dry");

const __FlashStringHelper* AsEcMeterI2C::calibrate_single_command_ =
    F("single");

const __FlashStringHelper* AsEcMeterI2C::calibrate_single_code_ = F("Cal,");

const __FlashStringHelper* AsEcMeterI2C::calibrate_double_low_command_ =
    F("double_low");

const __FlashStringHelper* AsEcMeterI2C::calibrate_double_low_code_ =
    F("Cal,low,");

const __FlashStringHelper* AsEcMeterI2C::calibrate_double_high_command_ =
    F("double_high");

const __FlashStringHelper* AsEcMeterI2C::calibrate_double_high_code_ =
    F("Cal,high,");

const __FlashStringHelper* AsEcMeterI2C::calibrate_clear_command_ = F("clear");

const __FlashStringHelper* AsEcMeterI2C::calibrate_clear_code_ = F("Cal,clear");

const __FlashStringHelper* AsEcMeterI2C::calibrate_check_code_ = F("Cal,?");

const __FlashStringHelper* AsEcMeterI2C::not_calibrated_code_ = F("?Cal,0");

const __FlashStringHelper* AsEcMeterI2C::sleep_code_ = F("Sleep");

const __FlashStringHelper* AsEcMeterI2C::unknown_command_error_ =
    F("Unknown command");

const __FlashStringHelper* AsEcMeterI2C::invalid_transition_error_ =
    F("Invalid transition");

const __FlashStringHelper* AsEcMeterI2C::receive_error_ = F("Receive failed");

const __FlashStringHelper* AsEcMeterI2C::not_calibrated_error_ =
    F("Failed calibration");

}  // namespace as_ec_meter
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box
