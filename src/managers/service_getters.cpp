#include "service_getters.h"

namespace inamata {
const __FlashStringHelper* ServiceGetters::network_nullptr_error_ =
    FPSTR("Network nullptr returned");

const __FlashStringHelper* ServiceGetters::server_nullptr_error_ =
    FPSTR("Server nullptr returned");
}  // namespace inamata