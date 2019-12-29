#include "connectivity.h"

namespace bernd_box {
namespace tasks {

CheckConnectivity::CheckConnectivity(
    Scheduler* scheduler, Network& network, Mqtt& mqtt, Io& io,
    const std::chrono::seconds wifi_connect_timeout,
    const uint mqtt_connection_attempts)
    : Task(scheduler),
      isSetup_(false),
      network_(network),
      mqtt_(mqtt),
      io_(io),
      wifi_connect_timeout_(wifi_connect_timeout),
      mqtt_connection_attempts_(mqtt_connection_attempts) {
  Task::setIterations(TASK_FOREVER);
  Task::setInterval(std::chrono::milliseconds(default_period_).count());
}

CheckConnectivity::~CheckConnectivity() {}

bool CheckConnectivity::OnEnable() { return Callback(); }

bool CheckConnectivity::Callback() {
  io_.setStatusLed(true);

  // If not connected to the WiFi, try to reconnect. Else reboot
  if (!network_.isConnected()) {
    if (network_.connect(wifi_connect_timeout_) == false) {
      Serial.println(
          F("CheckConnectivity::Callback: Failed to connect to WiFi.\n"
            "Restarting in 10s"));
      delay(10000);
      ESP.restart();
    }

    // Contact the server. If it fails there is something wrong. Do not proceed.
    String response = network_.pingSdgServer();
    if (response.isEmpty()) {
      Serial.println(
          F("CheckConnectivity::Callback: Failed to ping the server.\n"
            "Restarting in 10s"));
      delay(10000);
      ESP.restart();
    }

    // Set the clock. Used to timestamp measurements and check TLS certificates
    int error = network_.setClock(std::chrono::seconds(30));
    if (error) {
      Serial.println(
          F("CheckConnectivity::OnEnable: Failed to update time.\n"
            "Restarting in 10s"));
      delay(10000);
      ESP.restart();
    }
  }

  // If not connected to an MQTT broker, attempt to reconnect. Else reboot
  if (!mqtt_.isConnected()) {
    // Get the local MQTT broker's IP address and connect to it
    String local_ip_address = network_.getCoordinatorLocalIpAddress();
    if(local_ip_address.isEmpty()) {
      Serial.println(
          F("Failed to get coordinator's local IP address. Restarting in 10s"));
      ::delay(10000);
      ESP.restart();
    }

    int error = mqtt_.switchBroker(local_ip_address, ESPRandom::uuidToString(getUuid()));
    if(error) {
      Serial.println(
          F("Unable to connect to MQTT broker. Restarting in 10s"));
      ::delay(10000);
      ESP.restart();
    }

    // Notify the coordinator of our existance and capabilities
    mqtt_.sendRegister();
  }

  // Only receive after the actions have been registered
  if (isSetup_) {
    mqtt_.receive();
  }

  io_.setStatusLed(false);
  return true;
}
}  // namespace tasks
}  // namespace bernd_box
