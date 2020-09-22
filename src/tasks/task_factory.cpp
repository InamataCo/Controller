#include "task_factory.h"

namespace bernd_box {
namespace tasks {

TaskFactory::TaskFactory(Server& server, Scheduler& scheduler)
    : server_(server), scheduler_(scheduler) {}

bool TaskFactory::registerTask(const String& type, Factory factory) {
  return getFactories().insert({type, factory}).second;
}

BaseTask* TaskFactory::createTask(const JsonObjectConst& parameters) {
  const __FlashStringHelper* who = F(__PRETTY_FUNCTION__);

  JsonVariantConst type = parameters[F("type")];
  if (type.isNull() || !type.is<char*>()) {
    server_.sendError(who, "Missing property: type (string)");
    return nullptr;
  }

  // Check if a factory for the type exists. Then try to create such a task
  const auto& factory = getFactories().find(type);
  if (factory != getFactories().end()) {
    // Create a task via the respective task factory
    return factory->second(parameters, scheduler_);
  }

  server_.sendError(
      who, String(F("Could not find the factory type: ")) + type.as<String>());
  return nullptr;
}

const std::vector<String> TaskFactory::getFactoryNames() {
  std::vector<String> names;
  for(const auto& factory : getFactories()) {
    names.push_back(factory.first);
  }
  return names;
}

std::map<String, TaskFactory::Factory>& TaskFactory::getFactories() {
  static std::map<String, Factory> factories;
  return factories;
}

}  // namespace tasks
}  // namespace bernd_box
