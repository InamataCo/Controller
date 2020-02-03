#include "periphery_task_factory.h"

namespace bernd_box {
namespace periphery {

PeripheryTaskFactory::PeripheryTaskFactory(Mqtt& mqtt) : mqtt_(mqtt) {}

bool PeripheryTaskFactory::registerTask(const String& type,
                                        PeripheryTaskFactory::Factory factory) {
  return true;
}

void PeripheryTaskFactory::mqttCallback(char* topic, uint8_t* payload,
                                        unsigned int length) {
  // Extract the name from the topic and increment once to skip the last
  // slash: task/a48b109f-975f-42e2-9962-a6fb752a1b6e/add -> add
  char* action = strrchr(topic, '/');
  action++;
  if (strcmp(action, "start")) {
  } else if (strcmp(action, "stop")) {
  } else {
    mqtt_.sendError(F(__PRETTY_FUNCTION__),
                    String("Unknown task command [start, stop]: ") + action);
  }
}

bool PeripheryTaskFactory::createTask(std::shared_ptr<Periphery> periphery,
                                      const JsonObjectConst& parameters) {
  const char* who = __PRETTY_FUNCTION__;

  JsonVariantConst type = parameters[F("type")];
  if (type.isNull() || !type.is<char*>()) {
    mqtt_.sendError(who, "Missing property: type (string)");
    return false;
  }

  // Check if a factory for the type exists. Then try to create such a task
  const auto& factory = factories_.find(type);
  if(factory != factories_.end()) {
    auto task = factory->second(periphery, parameters);
    if (task) {
      const int task_id = getNextTaskId();
      tasks_.emplace(task_id, std::move(task));
    }
  }

  return true;
}

bool PeripheryTaskFactory::stopTask(const JsonObjectConst& parameter) {
  return false;
}

const int PeripheryTaskFactory::getNextTaskId() {
  // Use post-increment to increment after returing value. First value == 0
  return next_task_id_++;
}

std::map<String, PeripheryTaskFactory::Factory>
    PeripheryTaskFactory::factories_;

}  // namespace periphery
}  // namespace bernd_box
