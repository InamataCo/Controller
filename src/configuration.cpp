/**
 * All user-configurations are set here
 *
 * This includes Wifi's SSID and password.
 */

#include "configuration.h"

namespace inamata {

const __FlashStringHelper* wifi_portal_ssid = FPSTR("InamataSetup");
const __FlashStringHelper* wifi_portal_password = FPSTR("12345678");

StaticJsonDocument<JSON_PAYLOAD_SIZE> doc_in;
StaticJsonDocument<JSON_PAYLOAD_SIZE> doc_out;


}  // namespace inamata
