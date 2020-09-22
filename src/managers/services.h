#pragma once

#include <TaskSchedulerDeclarations.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

#include "configuration.h"
#include "managers/mqtt.h"
#include "managers/network.h"
#include "managers/server.h"
#include "managers/web_socket.h"
#include "peripheral/peripheral_controller.h"
#include "peripheral/peripheral_factory.h"
#include "tasks/task_controller.h"
#include "tasks/task_factory.h"

namespace bernd_box {

/**
 * All services provided as static instances by the middleware
 *
 * The services cover functions such as the connection to the server and MQTT
 * broker, the peripheral and task controller as well as the scheduler.
 */
class Services {
 public:
  static Network& getNetwork();
  static Mqtt& getMqtt();
  static Server& getServer();
  static peripheral::PeripheralController& getPeripheralController();
  static Scheduler& getScheduler();

 private:
  static Network network_;
  static Mqtt mqtt_;
  static WebSocket web_socket_;
  static WiFiClient wifi_client_;
  static Scheduler scheduler_;
  static peripheral::PeripheralController peripheral_controller_;
  static peripheral::PeripheralFactory peripheral_factory_;
  static tasks::TaskFactory task_factory_;
  static tasks::TaskController task_controller_;
};

}  // namespace bernd_box
