#ifndef BERND_BOX_LIBRARY_LIBRARY_H
#define BERND_BOX_LIBRARY_LIBRARY_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include <memory>

#include "config.h"
#include "managers/io_types.h"
#include "managers/mqtt.h"
#include "periphery/periphery.h"
#include "periphery/periphery_factory.h"
#include "periphery/periphery_task.h"

namespace bernd_box {
namespace library {

class Library {
 public:
  Library(Mqtt& mqtt, periphery::PeripheryFactory& periphery_factory);
  void handleCallback(char* topic, uint8_t* payload, unsigned int length);
  std::shared_ptr<periphery::Periphery> getPeriphery(const String& name);

 private:
  Result add(const JsonObjectConst& doc);
  Result remove(const JsonObjectConst& doc);
  Result execute(const JsonObjectConst& doc);

  Mqtt& mqtt_;
  std::map<String, std::shared_ptr<periphery::Periphery>> peripheries_;
  periphery::PeripheryFactory& periphery_factory_;
};

}  // namespace library
}  // namespace bernd_box

#endif
