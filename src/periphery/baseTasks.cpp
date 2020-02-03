#include "baseTasks.h"

#include "managers/services.h"

namespace bernd_box {
namespace periphery {

const String ErrorTask::TYPE = TASK_ERROR;

PeripheryTask& ErrorTaskFactory::createTask(
    std::shared_ptr<Periphery> periphery, const JsonObjectConst& doc) {
  return *new ErrorTask(periphery);
}

ErrorTask::ErrorTask(std::shared_ptr<Periphery> periphery)
    : PeripheryTask(periphery) {}

const __FlashStringHelper* ErrorTask::getType() { return type(); }

const __FlashStringHelper* ErrorTask::type() { return F("ErrorTask"); }

bool ErrorTask::Callback() {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);
  Services::getMqtt().sendError(who, String(F(ERROR_TASK_ERROR_MESSAGE)));
  return true;
}

const String ListOperationsTask::TYPE = TASK_ERROR;

PeripheryTask& ListOperationsTaskFactory::createTask(
    std::shared_ptr<Periphery> periphery, const JsonObjectConst& doc) {
  return *new ListOperationsTask(periphery);
}

const __FlashStringHelper* ListOperationsTask::getType() { return type(); }

const __FlashStringHelper* ListOperationsTask::type() {
  return F("ListOperationsTask");
}

ListOperationsTask::ListOperationsTask(std::shared_ptr<Periphery> periphery)
    : PeripheryTask(periphery) {}

bool ListOperationsTask::Callback() {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);
  auto values = getPeriphery()->getAvaiableTasks();
  DynamicJsonDocument doc(BB_MQTT_JSON_PAYLOAD_SIZE);

  // Create a list of all registered actions
  DynamicJsonDocument operations_doc(JSON_ARRAY_SIZE(values.size()));
  JsonArray operations_array = operations_doc.to<JsonArray>();

  for (const auto& operation : values) {
    operations_array.add(operation);
  }
  doc[TASK_LIST_RESULT_NODE] = operations_array;

  Services::getMqtt().send(who, doc);
  return true;
}

}  // namespace periphery
}  // namespace bernd_box
