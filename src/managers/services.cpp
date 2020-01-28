#include "services.h"

namespace bernd_box {

WiFiClient Services::wifi_client_;

Mqtt Services::mqtt_ = Mqtt(wifi_client_);

periphery::PeripheryFactory Services::periphery_factory_;

library::Library& Services::getLibrary() {
  return library::Library::getLibrary();
}

Mqtt& Services::getMqtt() { return mqtt_; }

periphery::PeripheryFactory& Services::getPeripheryFactory() {
  return periphery_factory_;
}

}  // namespace bernd_box
