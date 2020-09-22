#pragma once

#include <Arduino.h>

namespace bernd_box {

struct AccessPoint {
  const __FlashStringHelper* ssid;
  const __FlashStringHelper* password;
};

}