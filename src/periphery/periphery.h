#ifndef BERND_BOX_PERIPHERY_PERIPHERY_H
#define BERND_BOX_PERIPHERY_PERIPHERY_H

#include "library/library.h"
#include "managers/io.h"

#include <ArduinoJson.h>

namespace bernd_box {
namespace periphery {

using namespace bernd_box::library;

class TaskFactory;

class Periphery {
 public:
  virtual ~Periphery() = default;
  virtual const String& getType() = 0; 
  virtual TaskFactory& getTaskFactory(const JsonObjectConst& doc) = 0;
  virtual const bool isValid() = 0;
};

}  // namespace periphery
}  // namespace bernd_box

#endif