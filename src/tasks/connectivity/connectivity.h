#pragma once

#include <limits>

#include "TaskSchedulerDeclarations.h"
#include "configuration.h"
#include "managers/io.h"
#include "managers/mqtt.h"
#include "managers/network.h"
#include "managers/services.h"
#include "utils/setupNode.h"

namespace bernd_box {
namespace tasks {
namespace connectivity {

class CheckConnectivity : public Task {
 public:
  CheckConnectivity(Scheduler* scheduler);
  virtual ~CheckConnectivity();

 private:
  bool OnEnable() final;
  bool Callback() final;

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

  /**
   * Check the connection to the MQTT broker
   *
   * \deprecated As there are multiple methods to connect to the server
   * (WebSocket or MQTT), they now use the Server() interface.
   * \see handleServer()
   */
  bool checkMqtt();

  Network& network_;
  Mqtt& mqtt_;
  Server& server_;

  /// The MQTT receive callback is only enabled after the setup is complete
  bool is_setup_;
  /// Last time the internet time was checked
  long last_time_check_ms = std::numeric_limits<long>::max();
  /// Check the internet time every 24 hours
  long time_check_duration_ms = 24 * 60 * 60 * 1000;
};

}  // namespace connectivity
}  // namespace tasks
}  // namespace bernd_box
