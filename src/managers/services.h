#ifndef BERND_BOX_MANAGERS_SERVICES_H
#define BERND_BOX_MANAGERS_SERVICES_H

#include <WiFiClient.h>

#include "library/library.h"
#include "managers/mqtt.h"
#include "periphery/periphery_factory.h"

namespace bernd_box {

class Services {
 public:
  static Mqtt& getMqtt();
  // static Io& getIo();
  static bernd_box::library::Library& getLibrary();
  // static periphery::PeripheryFactory& getPeripheryFactory();

 private:
  static Mqtt mqtt_;
  static WiFiClient wifi_client_;
  static library::Library library_;
  // static Io io_;
  static periphery::PeripheryFactory periphery_factory_;

  static WiFiClient& getWifiClient();
};

}  // namespace bernd_box

#endif
