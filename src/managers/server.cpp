#include "server.h"

namespace bernd_box {

const __FlashStringHelper* Server::request_id_key_ = F("request_id");
const __FlashStringHelper* Server::type_key_ = F("type");
const __FlashStringHelper* Server::result_type_ = F("result");
const __FlashStringHelper* Server::telemetry_type_ = F("tel");
const __FlashStringHelper* Server::task_id_key_ = F("task_id");
const __FlashStringHelper* Server::system_type_ = F("sys");

}  // namespace bernd_box