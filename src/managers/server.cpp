#include "server.h"

namespace bernd_box {

const __FlashStringHelper* Server::request_id_key_ = F("request_id");
const __FlashStringHelper* Server::type_key_ = F("type");
const __FlashStringHelper* Server::type_result_name_ = F("result");

}  // namespace bernd_box