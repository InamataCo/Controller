#include "error_store.h"

namespace inamata {

String ErrorStore::genMissingProperty(String key, KeyType key_type) {
  String key_type_string;
  switch (key_type) {
    case KeyType::kUUID:
      key_type_string = uuid_type_;
    case KeyType::kString:
      key_type_string = string_type_;
      break;
    case KeyType::kBool:
      key_type_string = bool_type_;
      break;
    default:
      break;
  }

  // Create the missing property error string
  const size_t length =
      missing_property_length_a + key.length() + key_type_string.length();
  String error;
  error.reserve(length);
  error = missing_property_prefix_;
  error += key;
  error += F(" (");
  error += string_type_;
  error += F(")");

  return error;
}

String ErrorStore::genNotAValid(const utils::UUID& uuid, const String& type) {
  String error = uuid.toString();
  error += F(" not a valid ");
  error += type;
  return error;
}

const __FlashStringHelper* ErrorStore::missing_property_prefix_ =
    FPSTR("Missing property: ");
const __FlashStringHelper* ErrorStore::uuid_type_ = FPSTR("uuid");
const __FlashStringHelper* ErrorStore::uint32_t_type_ = FPSTR("uint32_t");
const __FlashStringHelper* ErrorStore::string_type_ = FPSTR("string");
const __FlashStringHelper* ErrorStore::bool_type_ = FPSTR("bool");

}  // namespace inamata