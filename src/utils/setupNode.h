#ifndef UTILS_SETUP_H
#define UTILS_SETUP_H

#include <FS.h>
#include <SPIFFS.h>

#include "ESPRandom.h"

#define FORMAT_SPIFFS_IF_FAILED true
#define UUID_PATH "/uuid.txt"
#define UUID_SIZE 16

bool setupNode() {
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("setupNode(): SPIFFS Mount Failed");
    return false;
  }
  // If the node does not have a UUID, generate one
  if (!SPIFFS.exists(UUID_PATH)) {
    std::vector<uint8_t> uuid = ESPRandom::uuid();
    
    String uuid_string(37);
    if (ESPRandom::uuidToString(uuid, uuid_string)) {
      Serial.println("setupNode(): Failed to convert UUID to string form");
      return false;
    }
    Serial.printf("Writing UUID (%s) to file: %s\n", uuid_string.c_str(),
                  UUID_PATH);
    fs::File uuid_file = SPIFFS.open(UUID_PATH);

    if (!uuid_file) {
      Serial.println("setupNode(): Failed to open file for writing UUID");
      return false;
    }
    if (!uuid_file.write(&uuid.front(), uuid.size())) {
      Serial.println("setupNode(): Failed to write UUID");
      return false;
    }
  }
  return true;
}

void getUuid() {
  fs::File uuid_file = SPIFFS.open(UUID_PATH);
  if (!uuid_file || uuid_file.size() != UUID_SIZE) {
    std::vector<uint8_t> uuid = ESPRandom::uuid();
  }
}

// bool getUuidString(String& uuid_string) {
//   fs::File uuid_file = SPIFFS.open(UUID_PATH);
//   if (!uuid_file) {
//     std::vector<uint8_t> uuid_buffer(16);
//     ESPRandom::uuid(uuid_buffer);
//     ESPRandom::uuidToString()
//   }
// }

#endif
