#ifndef BERND_BOX_MANAGERS_SERVICES_H
#define BERND_BOX_MANAGERS_SERVICES_H

#include <WiFiClient.h>
#include <TaskSchedulerDeclarations.h>

#include "library/library.h"
#include "managers/mqtt.h"
#include "peripheral/periphery_factory.h"
#include "tasks/task_controller.h"
#include "tasks/task_factory.h"

namespace bernd_box {

using namespace bernd_box::library;

class Services {
 public:
  static Mqtt& getMqtt();
  // static Io& getIo();
  static Library& getLibrary();
  // static peripheral::PeripheryFactory& getPeripheryFactory();
  static Scheduler& getScheduler();

 private:
  static Mqtt mqtt_;
  static WiFiClient wifi_client_;
  static Library library_;
  static Scheduler scheduler_;
  // static Io io_;
  static peripheral::PeripheralFactory periphery_factory_;
  static tasks::TaskFactory task_factory_;
  static tasks::TaskController task_controller_;

  static WiFiClient& getWifiClient();
};

}  // namespace bernd_box

#endif
