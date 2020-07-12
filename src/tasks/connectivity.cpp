#include "connectivity.h"

namespace bernd_box {
namespace tasks {

CheckConnectivity::CheckConnectivity(
    Scheduler* scheduler, Network& network, Io& io,
    const std::chrono::seconds wifi_connect_timeout,
    const uint mqtt_connection_attempts)
    : Task(scheduler),
      network_(network),
      mqtt_(Services::getMqtt()),
      server_(Services::getServer()),
      io_(io),
      wifi_connect_timeout_(wifi_connect_timeout),
      mqtt_connection_attempts_(mqtt_connection_attempts),
      is_setup_(false) {
  Task::setIterations(TASK_FOREVER);
  Task::setInterval(std::chrono::milliseconds(default_period_).count());
}

CheckConnectivity::~CheckConnectivity() {}

bool CheckConnectivity::OnEnable() {
  bool success = Callback();
  is_setup_ = success;
  return success;
}

bool CheckConnectivity::Callback() {
  io_.setStatusLed(true);

  CheckNetwork();
  CheckInternetTime();
  // CheckMqtt();
  HandleServer();

  io_.setStatusLed(false);
  return true;
}

bool CheckConnectivity::CheckNetwork() {
  if (!network_.isConnected()) {
    if (network_.connect(wifi_connect_timeout_) == false) {
      Serial.println(
          F("CheckConnectivity::Callback: Failed to connect to WiFi.\n"
            "Restarting in 10s"));
      delay(10000);
      ESP.restart();
    }

    // Contact the server. If it fails there is something wrong. Do not proceed.
    // String response = network_.pingSdgServer();
    // if (response.isEmpty()) {
    //   Serial.println(
    //       F("CheckConnectivity::Callback: Failed to ping the server.\n"
    //         "Restarting in 10s"));
    //   delay(10000);
    //   ESP.restart();
    // }
  }

  return true;
}

bool CheckConnectivity::CheckInternetTime() {
  if (millis() - last_time_check_ms > time_check_duration_ms) {
    last_time_check_ms = millis();

    int error = network_.setClock(std::chrono::seconds(30));
    if (error) {
      Serial.println(
          F("CheckConnectivity::OnEnable: Failed to update time.\n"
            "Restarting in 10s"));
      delay(10000);
      ESP.restart();
    }
  }

  return true;
}

bool CheckConnectivity::CheckMqtt() {
  // If not connected to an MQTT broker, attempt to reconnect. Else reboot
  if (!mqtt_.isConnected()) {
    // Get the local MQTT broker's IP address and connect to it
    String local_ip_address = network_.getCoordinatorLocalIpAddress();
    if (local_ip_address.isEmpty()) {
      Serial.println(
          F("Failed to get coordinator's local IP address. Restarting in 10s"));
      ::delay(10000);
      ESP.restart();
    }

    int error = mqtt_.switchBroker(local_ip_address,
                                   ESPRandom::uuidToString(getUuid()));
    if (error) {
      Serial.println(F("Unable to connect to MQTT broker. Restarting in 10s"));
      ::delay(10000);
      ESP.restart();
    }

    // Notify the coordinator of our existance and capabilities
    mqtt_.sendRegister();
  }

  // Only receive after the actions have been registered
  if (is_setup_) {
    mqtt_.receive();
  }

  return true;
}

bool CheckConnectivity::HandleServer() {
  if (!server_.IsConnected()) {
    if (!server_.Connect()) {
      Serial.println(F("Unable to connect to server. Restarting in 10s"));
      ::delay(10000);
      ESP.restart();
    } else {
      Serial.println(F("CheckConnectivity: Reconnected to server"));
    }
  }

  server_.Loop();

  return true;
}

}  // namespace tasks
}  // namespace bernd_box
