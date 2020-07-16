#include "setupNode.h"

namespace bernd_box {

std::vector<uint8_t> getUuid() {
  const char* uuid_path = "/uuid.txt";
  const uint uuid_size = 16;

  // Try to read the UUID from SPIFFS. If it does not exist, generate a new UUID
  fs::File uuid_file = SPIFFS.open(uuid_path, FILE_READ);
  if (!uuid_file) {
    std::vector<uint8_t> uuid = ESPRandom::uuid();

    // Close the file, to open it again in write mode to be able to save the new
    // UUID
    uuid_file.close();
    uuid_file = SPIFFS.open(uuid_path, FILE_WRITE);

    if (!uuid_file.write(uuid.data(), uuid.size())) {
      Serial.println("getUuid(): Failed to write UUID to SPIFFS");
    }
    uuid_file.close();
    return uuid;
  }

  // Read the UUID from SPIFFS
  std::vector<uint8_t> uuid = std::vector<uint8_t>(16);
  size_t bytes_read = uuid_file.read(uuid.data(), uuid.size());

  // Check if the UUID read from SPIFFS is valid
  if (!ESPRandom::isValidV4Uuid(uuid) || uuid_file.size() != uuid_size ||
      bytes_read != uuid_size) {
    // If it is invalid, print what was read an generate a new UUID
    String invalid_uuid;
    invalid_uuid.reserve(uuid.size() * 2 + 1);
    for (int i = 0; i < uuid.size(); i++) {
      invalid_uuid += String(uuid[i]);
      invalid_uuid += "-";
    }
    Serial.printf(
        "getUuid(): UUID (%s) read from SPIFFS (%d bytes) was not valid\n",
        invalid_uuid.c_str(), uuid_file.size());

    // Continue with a new UUID
    uuid = ESPRandom::uuid();

    // Close the file, to open it again in write mode to be able to save the new
    // UUID
    uuid_file.close();
    uuid_file = SPIFFS.open(uuid_path, FILE_WRITE);

    if (!uuid_file.write(uuid.data(), uuid.size())) {
      Serial.println("getUuid(): Failed to write UUID to SPIFFS");
    }
    uuid_file.close();
  }
  return uuid;
}

bool setupNode() {
  bool format_spiffs_if_failed = true;

  // Enable serial communication and prints
  Serial.begin(115200);

  // Initialize the file system
  if (!SPIFFS.begin(format_spiffs_if_failed)) {
    Serial.println("setupNode(): SPIFFS Mount Failed");
    return false;
  }

  // Print the node's UUID
  std::vector<uint8_t> uuid = getUuid();
  Serial.println(ESPRandom::uuidToString(uuid));
  return true;
}

}
