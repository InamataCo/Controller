#ifndef BERND_BOX_TASKS_CONNECTIVITY_H
#define BERND_BOX_TASKS_CONNECTIVITY_H

#include "task.h"

#include "io.h"
#include "mqtt.h"
#include "network.h"

namespace bernd_box {
namespace tasks {

class CheckConnectivity : public Task {
 public:
  CheckConnectivity(Scheduler* scheduler, Network& network, Mqtt& mqtt, Io& io,
                    const std::chrono::seconds wifi_connect_timeout,
                    const uint mqtt_connection_attempts);
  virtual ~CheckConnectivity();

  void now();

 private:
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