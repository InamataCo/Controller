#ifndef BERND_BOX_PERIPHERY_PERIPHERY_H
#define BERND_BOX_PERIPHERY_PERIPHERY_H

#include <ArduinoJson.h>

namespace bernd_box {
namespace periphery {

// using namespace bernd_box::library;

class TaskFactory;

class Periphery {
 public:
  virtual ~Periphery() = default;
  
  virtual const __FlashStringHelper* getType() = 0; 
  virtual TaskFactory& getTaskFactory(const JsonObjectConst& doc) = 0;
  virtual const bool isValid() = 0;
};

}  // namespace periphery
}  // namespace bernd_box

#endif
