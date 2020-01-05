#ifndef BERND_BOX_TASKS_CONNECTIVITY_H
#define BERND_BOX_TASKS_CONNECTIVITY_H

#include "managers/io.h"
#include "managers/mqtt.h"
#include "managers/network.h"
#include "task.h"
#include "utils/setupNode.h"

namespace bernd_box {
namespace tasks {

class CheckConnectivity : public Task {
 private:
  const std::chrono::milliseconds default_period_{100};

 public:
  CheckConnectivity(Scheduler* scheduler, Network& network, Mqtt& mqtt, Io& io,
                    const std::chrono::seconds wifi_connect_timeout,
                    const uint mqtt_connection_attempts);
  virtual ~CheckConnectivity();

  /// The MQTT receive callback is only enabled after the setup is complete
  bool isSetup_;

 private:
  bool OnEnable() final;
  bool Callback() final;

  Network& network_;
  Mqtt& mqtt_;
  Io& io_;
  const std::chrono::seconds wifi_connect_timeout_;
  const uint mqtt_connection_attempts_;
};

}  // namespace tasks
}  // namespace bernd_box

#endif
