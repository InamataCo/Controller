#pragma once

#include <TaskSchedulerDeclarations.h>

#include <limits>

#include "managers/service_getters.h"
#include "tasks/base_task.h"
#include "utils/chrono_abs.h"

namespace bernd_box {
namespace tasks {
namespace connectivity {

class CheckConnectivity : public BaseTask {
 public:
  CheckConnectivity(const ServiceGetters& services, Scheduler& scheduler);
  virtual ~CheckConnectivity();

  const String& getType() const final;
  static const String& type();

 private:
  bool OnTaskEnable() final;
  bool TaskCallback() final;

  /**
   * Connects to a network
   *
   * Attempts to connect to a known WiFi access point. If it fails, restart.
   *
   * \return True if all is ok
   */
  bool checkNetwork();

  /**
   * Performs time synchronization when necessary
   *
   * Attempts to perform time synchronization with an NTP server. If it fails,
   * restart.
   *
   * \return True if all is ok
   */
  bool checkInternetTime();

  /**
   * Performs server communication processing and ensure connected state
   *
   * Attempts to connect within a timeout. If it fails, restart. On success send
   * a registeration message.
   *
   * \return True if all is ok
   */
  bool handleServer();

  ServiceGetters services_;
  std::shared_ptr<Network> network_;
  std::shared_ptr<Server> server_;

  /// Last time the internet time was checked
  std::chrono::steady_clock::time_point last_time_check_ =
      std::chrono::steady_clock::time_point::max();
  const std::chrono::steady_clock::duration time_check_duration_ =
      std::chrono::hours(24);
};

}  // namespace connectivity
}  // namespace tasks
}  // namespace bernd_box
