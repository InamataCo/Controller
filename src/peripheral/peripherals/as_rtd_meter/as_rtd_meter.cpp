#include "as_rtd_meter.h"

#include "peripheral/peripheral_factory.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace as_rtd_meter {

AsRtdMeterI2C::AsRtdMeterI2C(const JsonVariantConst& parameters)
    : I2CAbstractPeripheral(parameters), Ezo_board(0) {
  // If the base class constructor failed, abort the constructor
  if (!isValid()) {
    return;
  }

  // Get and check the data point type for temperature readings
  data_point_type_ = utils::UUID(parameters[data_point_type_key_]);
  if (!data_point_type_.isValid()) {
    setInvalid(data_point_type_key_error_);
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

  // Set to sleep mode
  send_cmd(String(sleep_code_).c_str());
}

const String& AsRtdMeterI2C::getType() const { return type(); }

const String& AsRtdMeterI2C::type() {
  static const String name{"AsRtdMeterI2C"};
  return name;
}

capabilities::StartMeasurement::Result AsRtdMeterI2C::startMeasurement(
    const JsonVariantConst& parameters) {
  // Request a reading
  send_read_cmd();

  // Invalidate the last reading
  last_reading_ = NAN;

  return {.wait = reading_duration_};
}

capabilities::StartMeasurement::Result AsRtdMeterI2C::handleMeasurement() {
  // Receive reading values, check if errors occured, check if measurement has
  // stabilized. Repeat if not stable.
  Ezo_board::errors error = receive_read_cmd();
  if (error == Ezo_board::errors::SUCCESS) {
    // On success, store the reading an end the measurement process
    last_reading_ = reading;
    return {.wait = {}};
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

capabilities::GetValues::Result AsRtdMeterI2C::getValues() {
  // Use temperature reading of last reading. Use startMeasurement capability.
  // Invalidate reading after returning it.
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

const __FlashStringHelper* AsRtdMeterI2C::sleep_code_ = F("Sleep");

}  // namespace as_rtd_meter
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box