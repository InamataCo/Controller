#include "services.h"

namespace bernd_box {

Mqtt Services::mqtt_ =
    Mqtt(wifi_client_,
         std::bind(&periphery::PeripheryFactory::getFactoryNames,
                   &periphery_factory_),
         std::bind(&library::Library::handleCallback, &library_, _1, _2, _3),
         std::bind(&library::Library::taskCallback, &library_, _1, _2, _3));

WiFiClient Services::wifi_client_;

library::Library Services::library_ =
    library::Library(mqtt_, periphery_factory_, periphery_task_factory_);

Scheduler Services::scheduler_;

// Io Services::io_ = Io(mqtt_);

periphery::PeripheryFactory Services::periphery_factory_ =
    periphery::PeripheryFactory(mqtt_);

periphery::PeripheryTaskFactory Services::periphery_task_factory_(mqtt_);

library::Library& Services::getLibrary() { return library_; }

Mqtt& Services::getMqtt() { return mqtt_; }

Scheduler& Services::getScheduler() { return scheduler_; }

// periphery::PeripheryFactory& Services::getPeripheryFactory() {
//   return periphery_factory_;
// }

}  // namespace bernd_box
