#include "baseTasks.h"

#include "managers/services.h"

namespace bernd_box {
namespace periphery {

const String ErrorTask::TYPE = TASK_ERROR;

PeripheryTask& ErrorTaskFactory::createTask(
    std::shared_ptr<Periphery> periphery,
    const JsonObjectConst& doc) {
  return *new ErrorTask(periphery);
}

ErrorTask::ErrorTask(std::shared_ptr<Periphery> periphery)
    : PeripheryTask(periphery) {}

bool ErrorTask::Callback() {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);
  Services::getMqtt().sendError(who, String(F(ERROR_TASK_ERROR_MESSAGE)));
  disable();
  return true;
}

const String ListOperationsTask::TYPE = TASK_ERROR;

PeripheryTask& ListOperationsTaskFactory::createTask(
    std::shared_ptr<Periphery> periphery,
    const JsonObjectConst& doc) {
  return *new ListOperationsTask(periphery);
}

ListOperationsTask::ListOperationsTask(std::shared_ptr<Periphery> periphery)
    : PeripheryTask(periphery) {}

bool ListOperationsTask::Callback() {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);
  DynamicJsonDocument doc(BB_MQTT_JSON_PAYLOAD_SIZE);
  const JsonArray& result = doc.createNestedArray(F("result"));

  /*for (const auto& iterator : getPeriphery()->getTaskNames()) {
    result[F("name")] = iterator->first;
  }*/

  Services::getMqtt().send(who, doc);
  disable();
  return true;
}

}  // namespace periphery
}  // namespace bernd_box