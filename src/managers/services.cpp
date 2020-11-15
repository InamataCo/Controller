#include "services.h"

namespace bernd_box {

Network& Services::getNetwork() { return network_; }

peripheral::PeripheralController& Services::getPeripheralController() {
  return peripheral_controller_;
}

Mqtt& Services::getMqtt() { return mqtt_; }

Server& Services::getServer() { return web_socket_; }

Scheduler& Services::getScheduler() { return scheduler_; }

Network Services::network_{bernd_box::access_points, bernd_box::core_domain,
                           bernd_box::root_cas};

Mqtt Services::mqtt_{
    wifi_client_,
    std::bind(&peripheral::PeripheralFactory::getFactoryNames,
              &peripheral_factory_),
    std::bind(&peripheral::PeripheralController::handleCallback,
              &peripheral_controller_, _1),
    std::bind(&tasks::TaskController::handleCallback, &task_controller_, _1)};

WebSocket Services::web_socket_{
    std::bind(&peripheral::PeripheralController::getPeripheralIDs,
              &peripheral_controller_),
    std::bind(&peripheral::PeripheralController::handleCallback,
              &peripheral_controller_, _1),
    std::bind(&tasks::TaskController::getTaskIDs, &task_controller_),
    std::bind(&tasks::TaskController::handleCallback, &task_controller_, _1)
};

WiFiClient Services::wifi_client_;

Scheduler Services::scheduler_;

peripheral::PeripheralFactory Services::peripheral_factory_{web_socket_};

peripheral::PeripheralController Services::peripheral_controller_{
    web_socket_, peripheral_factory_};

tasks::TaskFactory Services::task_factory_{web_socket_, scheduler_};

tasks::TaskController Services::task_controller_{scheduler_, task_factory_,
                                                 web_socket_};

tasks::TaskRemovalTask Services::task_removal_task_{scheduler_, getServer()};

}  // namespace bernd_box
