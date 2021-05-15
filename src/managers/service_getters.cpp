#include "service_getters.h"

namespace bernd_box {
const __FlashStringHelper* ServiceGetters::nullptr_error_ =
    F("Nullptr returned");

const __FlashStringHelper* ServiceGetters::network_nullptr_error_ =
    F("Network nullptr returned");

const __FlashStringHelper* ServiceGetters::server_nullptr_error_ =
    F("Server nullptr returned");
}  // namespace bernd_box