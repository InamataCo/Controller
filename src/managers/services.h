#ifndef BERND_BOX_MANAGERS_SERVICES_H
#define BERND_BOX_MANAGERS_SERVICES_H

#include <TaskSchedulerDeclarations.h>
#include <WiFiClient.h>

#include "managers/mqtt.h"
#include "peripheral/peripheral_controller.h"
#include "peripheral/periphery_factory.h"
#include "tasks/task_controller.h"
#include "tasks/task_factory.h"

namespace bernd_box {

using namespace bernd_box::peripheral;

class Services {
 public:
  static Mqtt& getMqtt();
  // static Io& getIo();
  static PeripheralController& getLibrary();
  // static peripheral::PeripheryFactory& getPeripheryFactory();
  static Scheduler& getScheduler();

 private:
  static Mqtt mqtt_;
  static WiFiClient wifi_client_;
  static PeripheralController library_;
  static Scheduler scheduler_;
  // static Io io_;
  static peripheral::PeripheralFactory periphery_factory_;
  static tasks::TaskFactory task_factory_;
  static tasks::TaskController task_controller_;

  static WiFiClient& getWifiClient();
};

}  // namespace bernd_box

#endif
