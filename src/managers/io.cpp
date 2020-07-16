#include "io.h"

namespace bernd_box {

Io::Io(Mqtt& mqtt)
    : mqtt_(mqtt), one_wire_(one_wire_pin_), dallas_(&one_wire_) {
  pwm_channels_.fill(-1);
}

Io::~Io() {}

Result Io::init() {
  Result result = Result::kSuccess;
  // Set the status LED and pump GPIO to output
  pinMode(status_led_pin_, OUTPUT);
  pinMode(pump_pin_, OUTPUT);
  pinMode(atx_power_pin_, OUTPUT);

  // Init all enable/disable pins for analog sensors to avoid floating
  for (const auto& it : adcs_) {
    pinMode(it.second.enable_pin_id, OUTPUT);
  }

  disableAllAnalog();

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

Result Io::addDevice(const String& name, int& id) {
  bool found = false;
  for (const auto& i : names_) {
    if (i.second == name) {
      found = true;
      break;
    }
  }

  if (!found) {
    // Increment the last ID given and assign the device name to it
    last_name_id_++;
    names_[last_name_id_] = name;
    id = last_name_id_;
    return Result::kSuccess;
  } else {
    return Result::kNameAlreadyExists;
  }
}

void Io::removeDevice(const int id) {
  names_.erase(id);
  return;
}

const String& Io::getName(const int id) {
  const auto& name = names_.find(id);
  if (name != names_.end()) {
    return name->second;
  } else {
    return empty_;
  }
}

const int Io::getId(const String& name) {
  for (const auto& i : names_) {
    if (i.second == name) {
      return i.first;
    }
  }
  return -1;
}

void Io::setStatusLed(bool state) {
  if (state == true) {
    digitalWrite(status_led_pin_, HIGH);
  } else {
    digitalWrite(status_led_pin_, LOW);
  }
}

float Io::read(int sensor_id) {
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

float Io::readAnalog(int sensor_id) {
  float value = NAN;

  auto it = adcs_.find(sensor_id);
  if (it != adcs_.end()) {
    enableAnalog(it->first);
    value = analogRead(it->second.pin_id);
    value *= it->second.scaling_factor;
  }

  return value;
}

float Io::readAnalogV(int sensor_id) {
  float value = readAnalog(sensor_id);
  if (!std::isnan(value) && analog_reference_v_ != 0 &&
      analog_raw_range_ != 0) {
    value *= analog_reference_v_ / analog_raw_range_;
  } else {
    value = NAN;
  }

  return value;
}

Result Io::enableAnalog(int sensor_id) {
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

void Io::disableAnalog(int sensor_id) {
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

void Io::enableOutput(uint8_t pin) { pinMode(pin, OUTPUT); }

void Io::setPinState(uint8_t pin, bool state) {
  if (state) {
    digitalWrite(pin, HIGH);
  } else {
    digitalWrite(pin, LOW);
  }
}

Result Io::setPinPwm(uint8_t pin, float percent) {
  // Reserve the first unused channel, setup the PWM generator and attach a pin
  bool pin_set = false;
  for (int i = 0; i < pwm_channels_.size(); i++) {
    if (pwm_channels_[i] == -1 || pwm_channels_[i] == pin) {
      pwm_channels_[i] = pin;
      ledcSetup(i, pwm_frequency_, pwm_resolution_);
      ledcAttachPin(pin, i);

      // 0 to 1^pwm_resolution - 1 : 0 to 100 %, 0 to 8095 for 13 bits
      uint32_t duty_cycle =
          float((1 << pwm_resolution_) - 1) * percent / float(100);
      ledcWrite(i, duty_cycle);

      pin_set = true;
      break;
    }
  }

  if (pin_set) {
    return Result::kSuccess;
  } else {
    return Result::kFailure;
  }
}

void Io::removePinPwm(uint8_t pin) {
  // Detacht the pin from the PWM generator and free the reservation
  for (int i = 0; i < pwm_channels_.size(); i++) {
    if (pwm_channels_[i] == pin) {
      ledcDetachPin(pin);
      pwm_channels_[i] = -1;
    }
  }
}

float Io::readDallasTemperature(int sensor_id) {
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

float Io::readBh1750Light(int sensor_id) {
  float lux = NAN;

  const auto& it = bh1750s_.find(sensor_id);

  // If the sensor was found in the registered list
  if (it != bh1750s_.end()) {
    lux = it->second.interface.readLightLevel();
  }

  return lux;
}

float Io::readMax44009Light(int sensor_id) {
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

float Io::readBme280Air(int sensor_id) {
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

Result Io::checkI2cAddress(uint8_t address, const int interface_id) {
  const auto interface = i2c_interfaces_.find(interface_id);
  if (interface == i2c_interfaces_.end()) {
    return Result::kIdNotFound;
  }

  bool found = interface->second.find(address);
  if (found) {
    return Result::kSuccess;
  } else {
    return Result::kDeviceDisconnected;
  }
}

const std::map<int, TwoWire&>& Io::getI2cInterfaces() {
  return i2c_interfaces_;
}

void Io::i2cMqttCallback(char* topic, uint8_t* payload, unsigned int length) {
  const char* who = __PRETTY_FUNCTION__;

  // Try to deserialize the message
  DynamicJsonDocument doc(BB_JSON_PAYLOAD_SIZE);
  const DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    mqtt_.sendError(who, String(F("Deserialize failed: ")) + error.c_str());
    return;
  }

  // Handle all known actions
  bool action_found = false;
  JsonVariantConst add = doc[F("add")];
  if (!add.isNull()) {
    addI2cInterface(add);
    action_found = true;
  }
  JsonVariantConst remove_doc = doc[F("remove")];
  if (!remove_doc.isNull()) {
    removeI2cInterface(remove_doc);
    action_found = true;
  }

  // If no known action is included, send an error
  if (!action_found) {
    mqtt_.sendError(who, F("No known action found [add, remove]"));
  }
}

Result Io::addI2cInterface(const JsonObjectConst& doc) {
  const char* who = __PRETTY_FUNCTION__;

  // Check that all properties are there and their types
  JsonVariantConst name = doc[F("name")];
  if (name.isNull() || !name.is<char*>()) {
    mqtt_.sendError(who, "Missing property: name (string)");
    return Result::kFailure;
  }

  JsonVariantConst scl_pin = doc[F("scl_pin")];
  if (!scl_pin.is<uint8_t>()) {
    mqtt_.sendError(who, "Missing property: scl_pin (uint)");
    return Result::kFailure;
  }

  JsonVariantConst sda_pin = doc[F("sda_pin")];
  if (!sda_pin.is<uint8_t>()) {
    mqtt_.sendError(who, "Missing property: sda_pin (uint)");
    return Result::kFailure;
  }

  int id;
  Result result = addDevice(name, id);
  if (result != Result::kSuccess) {
    mqtt_.sendError(who, String(F("Name already exists: ")) + name.as<char*>());
    return Result::kFailure;
  }

  // for(auto wire : {&Wire, &Wire1}) {
  //   // Check if the TwoWire instance is being used
  //   bool used = false;
  //   for(auto i : i2c_interfaces_) {
  //     if(wire == &i.second) {
  //       used = true;
  //     }
  //   }

  //   // If not, add and initialize it
  //   if(!used) {
  //     i2c_interfaces_.emplace(id, wire);
  //     wire->begin(sda_pin, scl_pin);
  //     return Result::kSuccess;
  //   }
  // }

  // Check which TwoWire instance is not being used, then register it and start
  bool added = false;
  std::array<TwoWire*, 2> wires{&Wire, &Wire1};
  for (auto wire : wires) {
    if (std::none_of(
            i2c_interfaces_.begin(), i2c_interfaces_.end(),
            [wire](std::pair<int, TwoWire&> j) { return wire == &j.second; })) {
      i2c_interfaces_.emplace(id, *wire);
      wire->begin(sda_pin, scl_pin);
      added = true;
      break;
    }
  }

  // If it was not added, return a failure
  if (added) {
    return Result::kSuccess;
  } else {
    mqtt_.sendError(who, F("Both TwoWire instances are already being used."));
    return Result::kFailure;
  }
}

Result Io::removeI2cInterface(const JsonObjectConst& doc) {
  const char* who = __PRETTY_FUNCTION__;

  JsonVariantConst name = doc[F("name")];
  if (name.isNull() || !name.is<char*>()) {
    if (who) {
      mqtt_.sendError(who, "Missing property: name (string)");
    }
    return Result::kFailure;
  }

  const int id = getId(name);
  i2c_interfaces_.erase(id);
  removeDevice(id);

  return Result::kSuccess;
}

}  // namespace bernd_box
