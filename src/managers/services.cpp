#include "services.h"

#include "configuration.h"

namespace inamata {

Services::Services() {
  ServiceGetters getters = getGetters();

  peripheral_controller_.setServices(getters);
  task_controller_.setServices(getters);
  task_removal_task_.setServices(getters);
#ifdef ESP32
  ota_updater_.setServices(getters);
#endif
}

std::shared_ptr<Network> Services::getNetwork() { return network_; }

void Services::setNetwork(std::shared_ptr<Network> network) {
  network_ = network;
}

std::shared_ptr<WebSocket> Services::getWebSocket() { return web_socket_; }

void Services::setWebSocket(std::shared_ptr<WebSocket> web_socket) {
  web_socket_ = web_socket;
}

std::shared_ptr<Storage> Services::getStorage() { return storage_; }

void Services::setStorage(std::shared_ptr<Storage> storage) {
  storage_ = storage;
}

peripheral::PeripheralController& Services::getPeripheralController() {
  return peripheral_controller_;
}

tasks::TaskController& Services::getTaskController() {
  return task_controller_;
}

#ifdef ESP32
OtaUpdater& Services::getOtaUpdater() { return ota_updater_; }
#endif

Scheduler& Services::getScheduler() { return scheduler_; }

ServiceGetters Services::getGetters() {
  ServiceGetters getters(std::bind(&Services::getNetwork, this),
                         std::bind(&Services::getWebSocket, this),
                         std::bind(&Services::getStorage, this));
  return getters;
}

Scheduler Services::scheduler_{};

peripheral::PeripheralFactory Services::peripheral_factory_{};

peripheral::PeripheralController Services::peripheral_controller_{
    peripheral_factory_};

tasks::TaskFactory Services::task_factory_{scheduler_};

tasks::TaskController Services::task_controller_{scheduler_, task_factory_};

tasks::TaskRemovalTask Services::task_removal_task_{scheduler_};

#ifdef ESP32
OtaUpdater Services::ota_updater_{scheduler_};
#endif

}  // namespace inamata
