#include "connectivity.h"

namespace bernd_box {
namespace tasks {

CheckConnectivity::CheckConnectivity(
    Scheduler* scheduler, Network& network, Mqtt& mqtt,
    const std::chrono::seconds wifi_connect_timeout,
    const uint mqtt_connection_attempts)
    : Task(scheduler),
      network_(network),
      mqtt_(mqtt),
      wifi_connect_timeout_(wifi_connect_timeout),
      mqtt_connection_attempts_(mqtt_connection_attempts) {}

CheckConnectivity::~CheckConnectivity() {}

bool CheckConnectivity::Callback() {
  // If not connected to the WiFi, try to reconnect. Else reboot
  if (!network_.isConnected()) {
    Serial.println("WiFi: Disconnected. Attempting to reconnect");
    if (network_.connect(wifi_connect_timeout_) == false) {
      Serial.printf("WiFi: Could not connect to %s. Restarting\n",
                    network_.getSsid().c_str());
      ESP.restart();
    }
  }

  // If not connected to an MQTT broker, attempt to reconnect. Else reboot
  if (!mqtt_.isConnected()) {
    Serial.println("MQTT: Disconnected. Attempting to reconnect");
    if (mqtt_.connect(mqtt_connection_attempts_) == false) {
      Serial.printf("MQTT: Could not connect to broker on %s. Restarting\n",
                    mqtt_.getBrokerAddress().c_str());
      ESP.restart();
    }
  }

  mqtt_.receive();

  return true;
}
}  // namespace tasks
}  // namespace bernd_box