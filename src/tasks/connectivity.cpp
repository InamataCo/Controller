#include "connectivity.h"

namespace bernd_box {
namespace tasks {

CheckConnectivity::CheckConnectivity(
    Scheduler* scheduler, Network& network, Mqtt& mqtt, Io& io,
    const std::chrono::seconds wifi_connect_timeout,
    const uint mqtt_connection_attempts)
    : Task(scheduler),
      network_(network),
      mqtt_(mqtt),
      io_(io),
      wifi_connect_timeout_(wifi_connect_timeout),
      mqtt_connection_attempts_(mqtt_connection_attempts) {}

CheckConnectivity::~CheckConnectivity() {}

void CheckConnectivity::now() { Callback(); }

bool CheckConnectivity::Callback() {
  io_.setStatusLed(true);

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

  io_.setStatusLed(false);

  return true;
}
}  // namespace tasks
}  // namespace bernd_box