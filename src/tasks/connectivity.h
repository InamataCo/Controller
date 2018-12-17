#ifndef BERND_BOX_TASKS_CONNECTIVITY_H
#define BERND_BOX_TASKS_CONNECTIVITY_H

#include "task.h"

#include "mqtt.h"
#include "network.h"

namespace bernd_box {
namespace tasks {

class CheckConnectivity : public Task {
 public:
  CheckConnectivity(Scheduler* scheduler, Network& network, Mqtt& mqtt,
                    const std::chrono::seconds wifi_connect_timeout,
                    const uint mqtt_connection_attempts);
  virtual ~CheckConnectivity();

  bool Callback() final;

 private:
  Network& network_;
  Mqtt& mqtt_;
  const std::chrono::seconds wifi_connect_timeout_;
  const uint mqtt_connection_attempts_;
};

}  // namespace tasks
}  // namespace bernd_box

#endif