/**
 * All user-configurations are set here
 *
 * This includes Wifi's SSID and password.
 */

#include "configuration.h"

namespace bernd_box {

// Check Connectivity Task
const std::chrono::milliseconds check_connectivity_period{200};

// WiFi
const std::chrono::seconds wifi_connect_timeout(30);

// Server certificate authorities TLS certificates
const std::chrono::seconds server_connect_timeout{30};

}  // namespace bernd_box
