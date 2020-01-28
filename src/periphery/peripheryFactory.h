#ifndef BERND_BOX_PERIPHERY_FACTORY_H
#define BERND_BOX_PERIPHERY_FACTORY_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include <memory>

#include "library/library.h"
#include "periphery.h"

namespace bernd_box {
namespace periphery {

class PeripheryFactory {
 public:
  using Factory = std::shared_ptr<Periphery> (*)(const JsonObjectConst& parameter);

  virtual ~PeripheryFactory() = default;

  static bool registerFactory(const String& name, Factory factory);

  std::shared_ptr<Periphery> createPeriphery(const JsonObjectConst& parameter);

  std::map<String, Factory>& getFactories();
  
 private:
  static std::map<String, Factory> factories_;
};

}  // namespace periphery
}  // namespace bernd_box

#endif
