#ifndef BERND_BOX_PERIPHERY_FACTORY_H
#define BERND_BOX_PERIPHERY_FACTORY_H

#include <Arduino.h>

namespace bernd_box{
    namespace periphery{

        class PeripheryFactory{
            private:

            public:
                static PeripheryFactory& getPeripheryFactory();
                Periphery& createPeriphery(const JsonObjectConst& parameter);
        };

    }
}

#endif