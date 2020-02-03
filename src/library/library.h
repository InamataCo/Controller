#ifndef BERND_BOX_LIBRARY_LIBRARY_H
#define BERND_BOX_LIBRARY_LIBRARY_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include <memory>
#include <map>

#include "config.h"
#include "managers/io_types.h"
#include "managers/mqtt.h"
#include "periphery/periphery.h"
#include "periphery/periphery_factory.h"
#include "periphery/periphery_task.h"
#include "periphery/periphery_task_factory.h"

namespace bernd_box {
namespace library {

class Library {
 public:
  Library(Mqtt& mqtt, periphery::PeripheryFactory& periphery_factory,
          periphery::PeripheryTaskFactory& periphery_task_factory);
  void handleCallback(char* topic, uint8_t* payload, unsigned int length);
  void taskCallback(char* topic, uint8_t* payload, unsigned int length);
  std::shared_ptr<periphery::Periphery> getPeriphery(const String& name);

 private:
  Result add(const JsonObjectConst& doc);
  Result remove(const JsonObjectConst& doc);
  Result execute(const JsonObjectConst& doc);

  Mqtt& mqtt_;
  std::map<String, std::shared_ptr<periphery::Periphery>> peripheries_;
  periphery::PeripheryFactory& periphery_factory_;
  periphery::PeripheryTaskFactory& periphery_task_factory_;

  const __FlashStringHelper* task_add_suffix = F("add");
  const __FlashStringHelper* task_remove_suffix = F("remove");
};

}  // namespace library
}  // namespace bernd_box

#endif
