#pragma once

#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

namespace sdg {

void setup_ota(const char* hostname, const char* password);

void handle_ota();

}  // namespace sdg
