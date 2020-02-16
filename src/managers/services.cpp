#include "services.h"

namespace bernd_box {

Mqtt Services::mqtt_{
    wifi_client_,
    std::bind(&peripheral::PeripheralFactory::getFactoryNames,
              &periphery_factory_),
    std::bind(&library::Library::handleCallback, &library_, _1, _2, _3),
    std::bind(&tasks::TaskController::mqttCallback, &task_controller_, _1, _2,
              _3)};

WiFiClient Services::wifi_client_;

library::Library Services::library_{mqtt_, periphery_factory_};

Scheduler Services::scheduler_;

// Io Services::io_ = Io(mqtt_);

peripheral::PeripheralFactory Services::periphery_factory_{mqtt_};

library::Library& Services::getLibrary() { return library_; }

Mqtt& Services::getMqtt() { return mqtt_; }

Scheduler& Services::getScheduler() { return scheduler_; }

tasks::TaskFactory Services::task_factory_{mqtt_, scheduler_};

tasks::TaskController Services::task_controller_{scheduler_, task_factory_,
                                                 mqtt_};

}  // namespace bernd_box
