#include "server.h"

namespace inamata {

const __FlashStringHelper* Server::request_id_key_ = FPSTR("request_id");
const __FlashStringHelper* Server::type_key_ = FPSTR("type");
const __FlashStringHelper* Server::result_type_ = FPSTR("result");
const __FlashStringHelper* Server::telemetry_type_ = FPSTR("tel");
const __FlashStringHelper* Server::task_id_key_ = FPSTR("task_id");
const __FlashStringHelper* Server::system_type_ = FPSTR("sys");

}  // namespace inamata