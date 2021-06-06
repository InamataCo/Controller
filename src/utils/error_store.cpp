#include "error_store.h"

namespace bernd_box {

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

const __FlashStringHelper* ErrorStore::missing_property_prefix_ =
    F("Missing property: ");
const __FlashStringHelper* ErrorStore::uuid_type_ = F("uuid");
const __FlashStringHelper* ErrorStore::uint32_t_type_ = F("uint32_t");
const __FlashStringHelper* ErrorStore::string_type_ = F("string");
const __FlashStringHelper* ErrorStore::bool_type_ = F("bool");

}  // namespace bernd_box