#pragma once

#include <ArduinoJson.h>
#include <FS.h>

#include "LittleFS.h"
#include "managers/services.h"
#include "managers/types.h"
#include "utils/error_store.h"

namespace inamata {

bool setupNode(Services& services);

}  // namespace inamata
