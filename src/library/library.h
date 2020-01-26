#ifndef BERND_BOX_LIBRARY_LIBRARY_H
#define BERND_BOX_LIBRARY_LIBRARY_H

#include "managers/io.h"
#include "managers/mqtt.h"
//#include "periphery/periphery.h"
//#include "periphery/peripheryFactory.h"

#include <Arduino.h>
#include <ArduinoJson.h>

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
  static Library library_;

 public:
  static Library& getLibrary(); 
  Library(Mqtt& mqtt);
  Result handleCallback(char* topic, uint8_t* payload, unsigned int length);
  std::shared_ptr<Periphery> getPeriphery(const String& name);
  Mqtt& getMQTT();

 private:
  Mqtt& mqtt_;
  std::map<String, std::shared_ptr<Periphery>> peripheries_;
};

}  // namespace library
}  // namespace bernd_box

#endif