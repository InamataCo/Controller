#include "peripheryFactory.h"

#include "peripheries/dummy/dummyPeriphery.h"

namespace bernd_box{
    namespace periphery{

        PeripheryFactory& PeripheryFactory::peripheryFactory_ =  *new PeripheryFactory(); 

        PeripheryFactory& PeripheryFactory::getPeripheryFactory(){
            return peripheryFactory_;
        }

        Periphery& PeripheryFactory::createPeriphery(Library& library, const String name, const JsonObjectConst& parameter){
            return *new peripheries::dummy::DummyPeriphery(library, name);
        }

    }
}