#include "connectivity.h"

#include "configuration.h"

namespace bernd_box {
namespace tasks {
namespace connectivity {

CheckConnectivity::CheckConnectivity(const ServiceGetters& services,
                                     Scheduler& scheduler)
    : BaseTask(scheduler), services_(services) {
  Task::setIterations(TASK_FOREVER);
  Task::setInterval(
      std::chrono::milliseconds(check_connectivity_period).count());
}

CheckConnectivity::~CheckConnectivity() {}

const String& CheckConnectivity::getType() const { return type(); }

const String& CheckConnectivity::type() {
  static const String name{"CheckConnectivity"};
  return name;
}

bool CheckConnectivity::OnTaskEnable() {
  network_ = services_.getNetwork();
  if (network_ == nullptr) {
    setInvalid(services_.network_nullptr_error_);
    return false;
  }
  server_ = services_.getServer();
  if (server_ == nullptr) {
    setInvalid(services_.server_nullptr_error_);
    return false;
  }

  return Callback();
}

bool CheckConnectivity::TaskCallback() {
  checkNetwork();
  checkInternetTime();
  handleServer();

  return true;
}

bool CheckConnectivity::checkNetwork() {
  if (!network_->isConnected()) {
    if (network_->connect(wifi_connect_timeout) == false) {
      Serial.println(
          F("CheckConnectivity::Callback: Failed to connect to WiFi.\n"
            "Restarting in 10s"));
      delay(10000);
      ESP.restart();
    }
  }

  return true;
}

bool CheckConnectivity::checkInternetTime() {
  if (utils::chrono_abs(std::chrono::steady_clock::now() - last_time_check_) >
      time_check_duration_) {
    last_time_check_ = std::chrono::steady_clock::now();

    int error = network_->setClock(std::chrono::seconds(30));
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

bool CheckConnectivity::handleServer() {
  if (!server_->isConnected()) {
    if (!server_->connect(server_connect_timeout)) {
      Serial.println(F("Unable to connect to server. Restarting in 10s"));
      ::delay(10000);
      ESP.restart();
    } else {
      server_->sendRegister();
      Serial.println(F("CheckConnectivity: Reconnected to server"));
    }
  }

  server_->handle();
  return true;
}

}  // namespace connectivity
}  // namespace tasks
}  // namespace bernd_box
