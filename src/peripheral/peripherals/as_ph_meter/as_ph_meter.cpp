#include "as_ph_meter.h"

#include "peripheral/peripheral_factory.h"

namespace bernd_box {
namespace peripheral {
namespace peripherals {
namespace as_ph_meter {

AsPhMeterI2C::AsPhMeterI2C(const JsonVariantConst& parameters)
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

const String& AsPhMeterI2C::getType() const { return type(); }

const String& AsPhMeterI2C::type() {
  static const String name{"AsPhMeterI2C"};
  return name;
}

capabilities::StartMeasurement::Result AsPhMeterI2C::startMeasurement(
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

capabilities::StartMeasurement::Result AsPhMeterI2C::handleMeasurement() {
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

capabilities::GetValues::Result AsPhMeterI2C::getValues() {
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

bool AsPhMeterI2C::isReadingStable() const {
  if (std::isnan(last_reading_)) {
    return false;
  }
  if (fabsf(last_reading_ - reading) / reading < stabalized_threshold_) {
    return true;
  } else {
    return false;
  }
}

std::shared_ptr<Peripheral> AsPhMeterI2C::factory(
    const ServiceGetters& services, const JsonObjectConst& parameters) {
  return std::make_shared<AsPhMeterI2C>(parameters);
}

bool AsPhMeterI2C::registered_ =
    PeripheralFactory::registerFactory(type(), factory);

bool AsPhMeterI2C::capability_get_values_ =
    capabilities::GetValues::registerType(type());

bool AsPhMeterI2C::capability_start_measurement_ =
    capabilities::StartMeasurement::registerType(type());

const __FlashStringHelper* AsPhMeterI2C::temperature_c_key_ =
    F("temperature_c");

const __FlashStringHelper* AsPhMeterI2C::temperature_c_key_error_ =
    F("Wrong property: temperature_c (float)");

const __FlashStringHelper* AsPhMeterI2C::sleep_code_ = F("Sleep");

}  // namespace as_ph_meter
}  // namespace peripherals
}  // namespace peripheral
}  // namespace bernd_box