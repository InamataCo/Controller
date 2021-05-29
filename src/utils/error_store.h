#include <Arduino.h>

namespace bernd_box {
class ErrorStore {
 public:
  enum class KeyType {
    kUUID,
    kUint32t,
    kString,
  };

  static String genMissingProperty(String key, KeyType key_type);

 private:
  /**
   * The length A consists of the prefix + padding + terminator
   * The length B is the length of the key
   * The lenght C is the length of the key type
   * An example below illustrates the 
   * 
   * Missing property: some_key (some_type)
   * |---------------->|------>|>|--------|>
   * A-----------------B-------A-C--------A-
   */ 
  static const size_t missing_property_length_a = 18 + 2 + 2;

  static const __FlashStringHelper* missing_property_prefix_;
  static const __FlashStringHelper* uuid_type_;
  static const __FlashStringHelper* uint32_t_type_;
  static const __FlashStringHelper* string_type_;
};
}  // namespace bernd_box
