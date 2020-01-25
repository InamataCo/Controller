#ifndef BERND_BOX_PERIPHERY_FACTORY_H
#define BERND_BOX_PERIPHERY_FACTORY_H

#include <memory>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "periphery.h"
#include "library/library.h"

namespace bernd_box{
    namespace periphery{

        class PeripheryFactory{
            private:
                static PeripheryFactory& peripheryFactory_;

            public:
                static PeripheryFactory& getPeripheryFactory();
                std::shared_ptr<Periphery> createPeriphery(Library& library, const String name, const JsonObjectConst& parameter);
        };

    }
}

#endif