#include "io.h"

namespace bernd_box {

Io::Io() : one_wire_(one_wire_pin_), dallas_(&one_wire_) {}

Io::~Io() {}

Result Io::init() {
  Result result = Result::kSuccess;
  // Set the status LED and pump GPIO to output
  pinMode(status_led_pin_, OUTPUT);
  pinMode(pump_pin_, OUTPUT);

  // Init all enable/disable pins for analog sensors to avoid floating
  for (const auto& it : adcs_) {
    pinMode(it.second.enable_pin_id, OUTPUT);
  }

  disableAllAnalog();

  // Start I2C for the relevant sensors
  Wire.begin(i2c_sda_pin_, i2c_scl_pin_);

  // Set the sense mode of the BH1750 sensors
  for (auto& it : bh1750s_) {
    it.second.interface.begin(it.second.mode);
  }

  // Check that the addresses match and then init the BME280 sensors
  for (auto& it : bme280s_) {
    if (bme280sensors_.find(it.second.address) == bme280sensors_.end()) {
      Serial.printf(
          "BME280: No matching sensor found for address (0x%X) that is used by "
          "a sensor parameter in bme280s_.\n",
          it.second.address);
      result = Result::kFailure;
    }
  }
  for (auto& it : bme280sensors_) {
    it.second.setI2CAddress(it.first);
    it.second.beginI2C(Wire);
  }

  // Start up the Dallas Temperature library in asynchronous mode
  Serial.println("Starting Dallas Temperature library in async mode");
  dallas_.begin();
  dallas_.setWaitForConversion(false);

  // Locate and set up devices on the bus
  auto dallas = dallases_.begin();
  for (int i = 0; dallas != dallases_.end(); dallas++, i++) {
    if (!dallas_.getAddress(dallas->second.address, i)) {
      Serial.printf("Unable to find Dallas sensor for device %i named %s\n", i,
                    dallas->second.name.c_str());
      result = Result::kFailure;
    } else {
      Serial.printf("Found Dallas sensor (%lX) for device %i named %s\n",
                    dallasAddressToInt(dallas->second.address), i,
                    dallas->second.name.c_str());
      dallas_.setResolution(dallas->second.address,
                            uint8_t(dallas->second.resolution), false);
      Serial.printf("Set resolution to %i bits\n",
                    int(dallas->second.resolution));
    }
  }

  return result;
}

void Io::setStatusLed(bool state) {
  if (state == true) {
    digitalWrite(status_led_pin_, HIGH);
  } else {
    digitalWrite(status_led_pin_, LOW);
  }
}

float Io::read(Sensor sensor_id) {
  // If no matching sensors are found, return NAN
  float value = NAN;

  // If no sensor has been found, check analog sensors
  if (value == NAN) {
    value = readAnalog(sensor_id);
  }

  // If no sensor has been found, check the temperature sensors
  if (value == NAN) {
    value = readDallasTemperature(sensor_id);
  }

  // If no sensor has been found, check the light sensors
  if (value == NAN) {
    value = readBh1750Light(sensor_id);
  }

  // If no sensor has been found, check the light sensors
  if (value == NAN) {
    value = readMax44009Light(sensor_id);
  }

  return value;
}

float Io::readAnalog(Sensor sensor_id) {
  float value = NAN;

  auto it = adcs_.find(sensor_id);
  if (it != adcs_.end()) {
    enableAnalog(it->first);
    value = analogRead(it->second.pin_id);
    value *= it->second.scaling_factor;
  }

  return value;
}

float Io::readAnalogV(Sensor sensor_id) {
  float value = readAnalog(sensor_id);
  if (!std::isnan(value) && analog_reference_v_ != 0 && analog_raw_range_ != 0) {
    value *= analog_reference_v_ / analog_raw_range_;
  } else {
    value = NAN;
  }

  return value;
}

Result Io::enableAnalog(Sensor sensor_id) {
  Result result;

  auto it = adcs_.find(sensor_id);
  if (it != adcs_.end()) {
    result = enableAnalog(it->second);
  } else {
    result = Result::kIdNotFound;
  }

  return result;
}

Result Io::enableAnalog(const AdcSensor& adc) {
  Result result = Result::kSuccess;

  if (adc.enable_pin_id >= 0) {
    digitalWrite(adc.enable_pin_id, HIGH);
  } else {
    result = Result::kInvalidPin;
  }

  return result;
}

void Io::enableAllAnalog() {
  for (const auto& it : adcs_) {
    enableAnalog(it.second);
  }
}

void Io::disableAnalog(Sensor sensor_id) {
  auto it = adcs_.find(sensor_id);
  if (it != adcs_.end()) {
    disableAnalog(it->second);
  }
}

void Io::disableAnalog(const AdcSensor& adc) {
  if (adc.enable_pin_id >= 0) {
    digitalWrite(adc.enable_pin_id, LOW);
  }
}

void Io::disableAllAnalog() {
  for (const auto& it : adcs_) {
    disableAnalog(it.second);
  }
}

Result Io::setPumpState(bool state) {
  Result result = Result::kSuccess;

  if (pump_pin_ >= 0) {
    if (state) {
      digitalWrite(pump_pin_, HIGH);
    } else {
      digitalWrite(pump_pin_, LOW);
    }
  } else {
    result = Result::kInvalidPin;
  }

  return result;
}

float Io::readDallasTemperature(Sensor sensor_id) {
  float temperature_c = NAN;

  auto it = dallases_.find(sensor_id);

  // If the sensor was found in the registered list
  if (it != dallases_.end()) {
    Result result = readDallasTemperature(it->second, false, temperature_c);

    switch (result) {
      case Result::kDeviceDisconnected:
        Serial.printf("Dallas sensor (%lx) with sensor ID %d not connected: ",
                      dallasAddressToInt(it->second.address),
                      static_cast<uint>(sensor_id));
        break;
      case Result::kNotReady:
        Serial.printf(
            "Dallas sensor (%lx) with sensor ID %d not ready (async mode): ",
            dallasAddressToInt(it->second.address),
            static_cast<uint>(sensor_id));
        break;
      default:
        break;
    }
  }

  return temperature_c;
}

Result Io::requestDallasTemperatureUpdate(const DallasSensor& sensor,
                                          int& millisToWait) {
  Result result = Result::kSuccess;

  if (dallas_.requestTemperaturesByAddress(sensor.address) == false) {
    result = Result::kDeviceDisconnected;
  }

  millisToWait = dallas_.millisToWaitForConversion(uint8_t(sensor.resolution));

  return result;
}

Result Io::readDallasTemperature(const DallasSensor& sensor, bool asyncMode,
                                 float& temperature_c) {
  Result result = Result::kSuccess;

  if (asyncMode) {
    dallas_.setWaitForConversion(false);
    if (dallas_.isConversionComplete() == false) {
      result = Result::kNotReady;
    }
  } else {
    dallas_.setWaitForConversion(true);
  }

  if (result == Result::kSuccess) {
    float temp_c = dallas_.getTempC(sensor.address);
    if (temp_c != DEVICE_DISCONNECTED_C) {
      temperature_c = temp_c;
    } else {
      temperature_c = NAN;
      result = Result::kDeviceDisconnected;
    }
  }

  return result;
}
// {
//   dallas_.requestTemperaturesByAddress(dallas->second.address);
//   Serial.println(dallas_.getTempC(dallas->second.address));
// }

// dallas_.setResolution(insideThermometer, 12, false);

// long startTime = millis();
// dallas_.requestTemperaturesByAddress(insideThermometer);

// float tempC = dallas_.getTempC(insideThermometer);
// Serial.print("Temp C: ");
// Serial.println(tempC);
// Serial.print("Resolution bits: ");
// Serial.println(dallas_.getResolution(insideThermometer));
// // dallas_.setResolution(insideThermometer, 12, true);
// Serial.println(millis() - startTime);

long Io::dallasAddressToInt(const uint8_t* address) {
  const int bytes = 8;
  const int bits_per_byte = 8;
  long result = 0;

  for (int i = 0; i < bytes; i++) {
    result += long(address[(bytes - 1) - i]) << bits_per_byte * i;
  }

  return result;
}

float Io::readBh1750Light(Sensor sensor_id) {
  float lux = NAN;

  const auto& it = bh1750s_.find(sensor_id);

  // If the sensor was found in the registered list
  if (it != bh1750s_.end()) {
    lux = it->second.interface.readLightLevel();
  }

  return lux;
}

float Io::readMax44009Light(Sensor sensor_id) {
  float lux = NAN;

  const auto& it = max44009s_.find(sensor_id);

  // If the sensor was found in the registered list
  if (it != max44009s_.end()) {
    lux = it->second.interface.getLux();
    int error = it->second.interface.getError();
    if (error != 0) {
      Serial.printf("MAX44009 error while reading: %d\n", error);
    }
  }

  return lux;
}

float Io::readBme280Air(Sensor sensor_id) {
  float value = NAN;

  const auto& it = bme280s_.find(sensor_id);

  // If the sensor parameter was found in the registered list
  if (it != bme280s_.end()) {
    const auto& interface = bme280sensors_.find(it->second.address);
    // If the sensor with the respective address exists
    if (interface != bme280sensors_.end()) {
      switch (it->second.parameter) {
        case Bme280Parameter::kAltitudeFeet:
          value = interface->second.readFloatAltitudeFeet();
          break;
        case Bme280Parameter::kAltitudeMeters:
          value = interface->second.readFloatAltitudeMeters();
          break;
        case Bme280Parameter::kHumidity:
          value = interface->second.readFloatHumidity();
          break;
        case Bme280Parameter::kPressure:
          value = interface->second.readFloatPressure();
          break;
        case Bme280Parameter::kTemperatureC:
          value = interface->second.readTempC();
          break;
        case Bme280Parameter::kTemperatureF:
          value = interface->second.readTempF();
          break;
        default:
          Serial.printf("BME280 parameter (%d) not handled by readBme280Air",
                        static_cast<int>(it->second.parameter));
          value = NAN;
      }
    }
  }

  return value;
}

}  // namespace bernd_box
