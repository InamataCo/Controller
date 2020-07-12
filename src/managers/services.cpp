#include "services.h"

namespace bernd_box {

Mqtt Services::mqtt_{
    wifi_client_,
    std::bind(&peripheral::PeripheralFactory::getFactoryNames,
              &periphery_factory_),
    std::bind(&peripheral::PeripheralController::handleCallback, &library_, _1,
              _2, _3),
    std::bind(&tasks::TaskController::mqttCallback, &task_controller_, _1, _2,
              _3)};

WebSocket Services::web_socket_{
    std::bind(&peripheral::PeripheralFactory::getFactoryNames,
              &periphery_factory_),
    std::bind(&peripheral::PeripheralController::handleCallback, &library_, _1,
              _2, _3),
    std::bind(&tasks::TaskController::mqttCallback, &task_controller_, _1, _2,
              _3)};

WiFiClient Services::wifi_client_;

peripheral::PeripheralController Services::library_{mqtt_, periphery_factory_};

Scheduler Services::scheduler_;

// Io Services::io_ = Io(mqtt_);

peripheral::PeripheralFactory Services::periphery_factory_{mqtt_};

peripheral::PeripheralController& Services::getLibrary() { return library_; }

Mqtt& Services::getMqtt() { return mqtt_; }

Server& Services::getServer() { return web_socket_; }

Scheduler& Services::getScheduler() { return scheduler_; }

tasks::TaskFactory Services::task_factory_{mqtt_, scheduler_};

tasks::TaskController Services::task_controller_{scheduler_, task_factory_,
                                                 mqtt_};

}  // namespace bernd_box
