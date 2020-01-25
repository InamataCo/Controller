#ifndef BERND_BOX_PERIPHERY_PERIPHERY_H
#define BERND_BOX_PERIPHERY_PERIPHERY_H

#include "library/library.h"
#include "managers/io.h"
#include "peripheryTask.h"

#include <ArduinoJson.h>

namespace bernd_box {
namespace periphery {

using namespace bernd_box::library;

class Periphery {
 public:
  virtual const String& getType() = 0; 
  virtual Result executeTask(const JsonObjectConst& doc);
  virtual Library& getLibrary();
};

}  // namespace periphery
}  // namespace bernd_box

#endif