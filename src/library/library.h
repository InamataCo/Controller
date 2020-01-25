#ifndef BERND_BOX_LIBRARY_LIBRARY_H
#define BERND_BOX_LIBRARY_LIBRARY_H

#include "managers/io.h"
#include "managers/mqtt.h"
//#include "periphery/periphery.h"
//#include "periphery/peripheryFactory.h"

#include <Arduino.h>
#include <ArduinoJson.h>

// STD C++ includes placed after Arduino.h
#include <algorithm>
#include <cmath>
#include <map>
#include <string>

namespace bernd_box {
namespace periphery {
class Periphery;
}
}  // namespace bernd_box

namespace bernd_box {
namespace library {

using namespace bernd_box::periphery;

class Library {
 private:
  Result add(const JsonObjectConst& doc);
  Result remove(const JsonObjectConst& doc);
  Result execute(const JsonObjectConst& doc);

 public:
  Library(Mqtt& mqtt);
  Result handleCallback(char* topic, uint8_t* payload, unsigned int length);
  Periphery& getPeriphery(String& name);
  Mqtt& getMQTT();

 private:
  Mqtt& mqtt_;
  std::map<String, Periphery&> peripheries_;
};
}  // namespace library
}  // namespace bernd_box

#endif