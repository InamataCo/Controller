#ifndef BERND_BOX_MANAGERS_SERVICES_H
#define BERND_BOX_MANAGERS_SERVICES_H

#include <WiFiClient.h>

#include "library/library.h"
#include "mqtt.h"

namespace bernd_box {

class Services {
 private:
  static Mqtt mqtt_;
  static WiFiClient wifiClient_;

  static WiFiClient& getWifiClient();

 public:
  static Mqtt& getMQTT();
  static library::Library& getLibrary();
};

}  // namespace bernd_box

#endif