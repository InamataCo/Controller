#pragma once

#include "TaskSchedulerDeclarations.h"
#include "managers/io.h"
#include "managers/mqtt.h"
#include "managers/network.h"
#include "managers/services.h"
#include "utils/setupNode.h"

namespace bernd_box {
namespace tasks {

class CheckConnectivity : public Task {
 private:
  const std::chrono::milliseconds default_period_{200};

 public:
  CheckConnectivity(Scheduler* scheduler, Network& network,
                    const std::chrono::seconds wifi_connect_timeout,
                    const uint mqtt_connection_attempts);
  virtual ~CheckConnectivity();

 private:
  bool OnEnable() final;
  bool Callback() final;

  bool checkNetwork();
  bool checkInternetTime();
  bool handleServer();
  bool checkMqtt();

  Network& network_;
  Mqtt& mqtt_;
  Server& server_;
  const std::chrono::seconds wifi_connect_timeout_;
  const uint mqtt_connection_attempts_;

  /// The MQTT receive callback is only enabled after the setup is complete
  bool is_setup_;
  /// Last time the internet time was checked
  long last_time_check_ms;
  /// Check the internet time every 24 hours
  long time_check_duration_ms = 24 * 60 * 60 * 1000;
};

}  // namespace tasks
}  // namespace bernd_box
