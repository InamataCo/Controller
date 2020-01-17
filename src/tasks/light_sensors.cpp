#include "light_sensors.h"

#include <array>
#include <type_traits>

namespace bernd_box {
namespace tasks {

template <typename T, std::size_t N>
constexpr bool is_sorted(std::array<T, N> const& arr, std::size_t from) {
  return N - from == 0 or
         (arr[from - 1] <= arr[from] and is_sorted(arr, from + 1));
}

template <typename T, std::size_t N>
constexpr bool is_sorted(std::array<T, N> const& arr) {
  return N == 0 or is_sorted(arr, 1);
}

LightSensors::LightSensors(Scheduler* scheduler, Io& io, Mqtt& mqtt)
    : Task(scheduler), io_(io), mqtt_(mqtt) {
  setIterations(TASK_FOREVER);
  Task::setInterval(std::chrono::milliseconds(default_period_).count());

  static_assert(is_sorted(bh1750_addresses_),
                "BH1750 addresses are not sorted.");
  static_assert(is_sorted(max44009_addresses_),
                "MAX44009 addresses are not sorted.");
}

LightSensors::~LightSensors() {}

bool LightSensors::OnEnable() {
  mqtt_.send("light_sensors_active", "true");

  return true;
}

bool LightSensors::Callback() {
  io_.setStatusLed(true);

  for (const auto& max44009 : io_.max44009s_) {
    float value = io_.readMax44009Light(max44009.first);
    Serial.printf("Ambient brightness (ID: %u) is %f %s\n",
                  static_cast<int>(max44009.first), value,
                  max44009.second.unit.c_str());
    mqtt_.send(max44009.second.name.c_str(), value);
  }

  io_.setStatusLed(false);

  return true;
}

void LightSensors::OnDisable() { mqtt_.send("light_sensors_active", "false"); }

void LightSensors::mqttCallback(char* topic, uint8_t* payload,
                                unsigned int length) {
  const char* who = __PRETTY_FUNCTION__;

  // Try to deserialize the message
  DynamicJsonDocument doc(BB_MQTT_JSON_PAYLOAD_SIZE);
  const DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    mqtt_.sendError(who, String(F("Deserialize failed: ")) + error.c_str());
    return;
  }

  // Handle all known actions
  bool action_found = false;
  JsonVariantConst find_sensors_doc = doc[F("find_sensors")];
  if (!find_sensors_doc.isNull()) {
    findSensors(find_sensors_doc);
    action_found = true;
  }
  JsonVariantConst add_sensor_doc = doc[F("add_sensor")];
  if (!add_sensor_doc.isNull()) {
    findSensors(add_sensor_doc);
    action_found = true;
  }

  // If no known action is included, send an error
  if (!action_found) {
    mqtt_.sendError(who, "No known action found.");
  }
}

Result LightSensors::findSensors(const JsonObjectConst& doc) {
  const char* who = __PRETTY_FUNCTION__;

  DynamicJsonDocument sensors_doc(BB_MQTT_JSON_PAYLOAD_SIZE);
  const JsonArray& sensors = sensors_doc.createNestedArray(F("result"));

  // Check all I2C interfaces if a device
  auto checkSensor = [](Io& io, const JsonArray& sensors, uint8_t address,
                        const __FlashStringHelper* type) {
    for (const auto& interface : io.getI2cInterfaces()) {
      Result result = io.checkI2cAddress(address, interface.first);
      if (result == Result::kSuccess) {
        const JsonObject& sensor = sensors.createNestedObject();
        sensor[F("i2c_interface")] = io.getName(interface.first);
        sensor[F("type")] = type;
        sensor[F("address")] = address;
      };
    }
  };

  for (const auto& bh1750_address : bh1750_addresses_) {
    checkSensor(io_, sensors, bh1750_address, sensor_types_.bh1750);
  }
  for (const auto& max44009_address : max44009_addresses_) {
    checkSensor(io_, sensors, max44009_address, sensor_types_.max44009);
  }

  std::array<const __FlashStringHelper*, 3> keys{
      {F("i2c_interface"), F("type"), F("address")}};

  // Check if all required properties have been defined
  for (const auto& sensor : sensors) {
    for (const auto& key : keys) {
      if (!sensor.containsKey(key)) {
        mqtt_.sendError(who, String(F("Failed serializing: ")) + key);
        return Result::kFailure;
      }
    }
  }

  mqtt_.send(name_, sensors_doc);
  return Result::kSuccess;
}

Result LightSensors::addSensor(const JsonObjectConst& doc) {
  const char* who = __PRETTY_FUNCTION__;

  // Check that the keys are present
  JsonVariantConst name = doc[F("name")];
  if (name.isNull() || !name.is<char*>()) {
    mqtt_.sendError(who, F("Missing property: name (string)"));
    return Result::kFailure;
  }

  JsonVariantConst i2c_interface_name = doc[F("i2c_interface")];
  if (i2c_interface_name.isNull() || !i2c_interface_name.is<char*>()) {
    mqtt_.sendError(who, F("Missing property: i2c_interface (string)"));
    return Result::kFailure;
  }
  const int i2c_interface_id = io_.getId(i2c_interface_name);
  if (i2c_interface_id < 0) {
    mqtt_.sendError(who, F("I2C interface name not found"));
    return Result::kFailure;
  }
  auto i2cs = io_.getI2cInterfaces();
  auto i2c_it = i2cs.find(i2c_interface_id);
  if (i2c_it == i2cs.end()) {
    mqtt_.sendError(who, F("I2C interface not found"));
  }

  JsonVariantConst type = doc[F("type")];
  if (type.isNull() || !type.is<char*>()) {
    mqtt_.sendError(who, F("Missing property: type (string)"));
    return Result::kFailure;
  }
  if (type != sensor_types_.bh1750 || type != sensor_types_.max44009) {
    mqtt_.sendError(who, String(F("Invalid type: ")) + type.as<String>() +
                             String(F(" [")) + sensor_types_.bh1750 +
                             String(F(", ")) + sensor_types_.max44009 +
                             String(F("]")));
    return Result::kFailure;
  }

  JsonVariantConst address = doc[F("address")];
  if (!address.is<int>()) {
    mqtt_.sendError(who, F("Missing property: address (int)"));
    return Result::kFailure;
  }

  // Check if the device exists
  Result result = io_.checkI2cAddress(i2c_interface_id, address);
  if (result != Result::kSuccess) {
    mqtt_.sendError(who, String(F("Device not foung on I2C interface: ")) +
                             String(address.as<int>(), HEX) + F(" : ") +
                             i2c_interface_name.as<String>());
    return Result::kFailure;
  }

  int id;
  result = io_.addDevice(name, id);
  if (result != Result::kSuccess) {
    mqtt_.sendError(who, String(F("Name already exists: ")) + name.as<char*>());
    return Result::kFailure;
  }

  if (type == sensor_types_.bh1750) {
    io_.bh1750s_[id] =
        Bh1750Sensor{BH1750(), address.as<uint8_t>(), i2c_interface_id};

    auto i2cs = io_.getI2cInterfaces();
    auto i2c_it = i2cs.find(i2c_interface_id);
    if (i2c_it != i2cs.end()) {
      io_.bh1750s_[id].interface.begin(BH1750::CONTINUOUS_HIGH_RES_MODE,
                                       address, &i2c_it->second);
    } else {
      mqtt_.sendError(who, F("Cannot find "));
    }
  }
  // MAX44009 library has to be updated to support setting the TwoWire interface
  //  else if (type == sensor_types_.max44009) {
  //   io_.max44009s_[id];
  // }
}

}  // namespace tasks
}  // namespace bernd_box
