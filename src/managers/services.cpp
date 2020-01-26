#include "services.h"

namespace bernd_box {

WiFiClient Services::wifiClient_ = WiFiClient();
Mqtt Services::mqtt_ = Mqtt(wifiClient_);

library::Library& Services::getLibrary() {
  return library::Library::getLibrary();
}


  Mqtt& Services::getMQTT(){
      return mqtt_;
  }

}  // namespace bernd_box