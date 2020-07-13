#include "task_factory.h"

namespace bernd_box {
namespace tasks {

TaskFactory::TaskFactory(Mqtt& mqtt, Scheduler& scheduler)
    : mqtt_(mqtt), scheduler_(scheduler) {}

bool TaskFactory::registerTask(const String& type, Factory factory) {
  return factories_.insert({type, factory}).second;
}

BaseTask* TaskFactory::createTask(const JsonObjectConst& parameters) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  JsonVariantConst type = parameters[F("type")];
  if (type.isNull() || !type.is<char*>()) {
    mqtt_.sendError(who, "Missing property: type (string)");
    return nullptr;
  }

  // Check if a factory for the type exists. Then try to create such a task
  const auto& factory = factories_.find(type);
  if (factory != factories_.end()) {
    // Create a task via the respective task factory
    return factory->second(parameters, scheduler_);
  }

  mqtt_.sendError(
      who, String(F("Could not find the factory type: ")) + type.as<String>());
  return nullptr;
}

const std::vector<String> TaskFactory::getTaskNames() {
  std::vector<String> names;
  for(const auto& factory : factories_) {
    names.push_back(factory.first);
  }
  return names;
}


std::map<String, TaskFactory::Factory> TaskFactory::factories_;

}  // namespace tasks
}  // namespace bernd_box
