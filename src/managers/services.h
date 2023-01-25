#pragma once

#include <WString.h>

#include <TaskSchedulerDeclarations.h>

#include "managers/service_getters.h"
#include "managers/network.h"
#include "managers/web_socket.h"
#ifdef ESP32
#include "managers/ota_updater.h"
#endif
#include "peripheral/peripheral_controller.h"
#include "peripheral/peripheral_factory.h"
#include "tasks/task_controller.h"
#include "tasks/task_factory.h"
#include "tasks/task_removal_task.h"

namespace inamata {

/**
 * All the system services
 *
 * The services cover functions such as the connection to the server,
 * the peripheral and task controller as well as the scheduler. The task and
 * peripheral services are static instances, while the network and server are
 * created by the setup procedure setupNode().
 */
class Services {
 public:
  /**
   * Set the service getters for the static services (task and peripheral
   * controller as well as task removal task).
   */
  Services();
  virtual ~Services() = default;

  std::shared_ptr<Network> getNetwork();
  void setNetwork(std::shared_ptr<Network> network);

  std::shared_ptr<WebSocket> getWebSocket();
  void setWebSocket(std::shared_ptr<WebSocket> web_socket);

  std::shared_ptr<Storage> getStorage();
  void setStorage(std::shared_ptr<Storage> storage);

  static peripheral::PeripheralController& getPeripheralController();
  static tasks::TaskController& getTaskController();
  #ifdef ESP32
  static OtaUpdater& getOtaUpdater();
  #endif

  static Scheduler& getScheduler();

  /**
   * Get callbacks to get the pointers to dynamic services (network and server)
   * 
   * \return Struct with callbacks to get pointers to the services
   */
  ServiceGetters getGetters();

 private:
  /// Handles network connectivity and time synchronization
  std::shared_ptr<Network> network_;
  /// Handles communication to the server
  std::shared_ptr<WebSocket> web_socket_;
  /// Handles FS / EEPROM storage
  std::shared_ptr<Storage> storage_;
  /// Executes the active tasks
  static Scheduler scheduler_;
  /// Creates peripherals with the registered peripheral factory callbacks
  static peripheral::PeripheralFactory peripheral_factory_;
  /// Handles server requests to create / delete peripherals
  static peripheral::PeripheralController peripheral_controller_;
  /// Creates tasks with the registered task factory callbacks
  static tasks::TaskFactory task_factory_;
  /// Handles server requests to start / stop tasks
  static tasks::TaskController task_controller_;
  /// Singleton to delete stopped tasks and inform the server
  static tasks::TaskRemovalTask task_removal_task_;
  /// Singleton to perform OTA updates
  #ifdef ESP32
  static OtaUpdater ota_updater_;
  #endif
};

}  // namespace inamata
