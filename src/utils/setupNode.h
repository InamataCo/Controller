#ifndef UTILS_SETUP_H
#define UTILS_SETUP_H

#include <FS.h>
#include <SPIFFS.h>
#include <vector>

#include "ESPRandom.h"

namespace bernd_box {

/**
 * Open the UUID file. If it does not exist, generate a new one and save it
 */
std::vector<uint8_t> getUuid();

bool setupNode();

}
#endif
