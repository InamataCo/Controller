#ifndef BERND_BOX_MANAGERS_SERVICES_H
#define BERND_BOX_MANAGERS_SERVICES_H

#include <WiFiClient.h>

#include "library/library.h"
#include "mqtt.h"
#include "periphery/peripheryFactory.h"

namespace bernd_box {

class Services {
 private:
  static Mqtt mqtt_;
  static WiFiClient wifi_client_;
  static periphery::PeripheryFactory periphery_factory_;

  static WiFiClient& getWifiClient();

 public:
  static Mqtt& getMqtt();
  static library::Library& getLibrary();
  static periphery::PeripheryFactory& getPeripheryFactory();
};

}  // namespace bernd_box

#endif
