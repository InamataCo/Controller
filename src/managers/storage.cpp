#include "storage.h"

namespace inamata {

DynamicJsonDocument& Storage::openSecrets() {
  if (secrets_doc_.capacity() != 0) {
    return secrets_doc_;
  }

#ifdef ESP32
  // Initialize the file system
  if (!LittleFS.begin()) {
    if (!LittleFS.begin(true)) {
      TRACELN(F("Failed mounting LittleFS"));
      return secrets_doc_;
    } else {
      TRACELN(F("Formatted LittleFS on fail"));
    }
  } else {
    // Load a common config file for the subsystems
    fs::File secrets_file = LittleFS.open(secrets_path_, "r+");
    if (secrets_file) {
      size_t file_size = secrets_file.size();
      size_t doc_size =
          file_size > min_doc_size_ ? file_size * 1.5 : min_doc_size_;
      secrets_doc_ = DynamicJsonDocument(doc_size);
      DeserializationError error = deserializeJson(secrets_doc_, secrets_file);
      secrets_file.close();
      if (error) {
        TRACELN(F("Failed parsing secrets.json"));
        closeSecrets(false);
      }
    } else {
      TRACELN(F("Failed opening secrets.json"));
    }
  }
#else
  EEPROM.begin(max_doc_size_);
  size_t file_size;
  // The first 4 bytes hold the size of the secrets.json file
  for (int i = 0; i < 4; i++) {
    *(reinterpret_cast<uint8_t*>(&file_size) + i) = EEPROM.read(i);
  }
  TRACEF("Size: %u\n", file_size);
  // If size not plausible, set to minimum size and return empty doc
  if (file_size > max_doc_size_) {
    secrets_doc_ = DynamicJsonDocument(min_doc_size_);
    TRACEF("Doc size: %u\n", secrets_doc_.capacity());
    return secrets_doc_;
  }
  size_t doc_size = fileToDocSize(file_size);
  secrets_doc_ = DynamicJsonDocument(doc_size);
  EepromStream eepromStream(4, file_size);
  deserializeJson(secrets_doc_, eepromStream);
  EEPROM.end();
#endif
  return secrets_doc_;
}

void Storage::closeSecrets(bool save) {
  if (save) {
    saveSecrets();
  }
  secrets_doc_ = DynamicJsonDocument(0);
}

void Storage::saveSecrets() {
#ifdef ESP32
  fs::File secrets_file = LittleFS.open(secrets_path_, "w");
  if (!secrets_file) {
    TRACEF("Failed open (w): %s\n", secrets_path_);
    return;
  }

  if (serializeJson(secrets_doc_, secrets_file) == 0) {
    TRACELN(F("Failed to write secrets"));
  }
  secrets_file.close();
  TRACELN(F("Saved secrets"));
#else
  size_t file_size = measureJson(secrets_doc_);
  if (file_size > max_doc_size_ - 4) {
    TRACELN(F("Secret file too large"));
    return;
  }
  EEPROM.begin(file_size + 4);
  TRACEF("file size: %u\n", file_size);
  for (int i = 0; i < 4; i++) {
    EEPROM.write(i, *(reinterpret_cast<uint8_t*>(&file_size) + i));
  }
  EepromStream eepromStream(4, file_size);
  size_t bytes_written = serializeJson(secrets_doc_, eepromStream);
  eepromStream.flush();
  TRACEF("Saved bytes: %u\n", bytes_written);
#endif
#ifdef ENABLE_TRACE
  serializeJson(secrets_doc_, Serial);
#endif
}

size_t Storage::fileToDocSize(size_t file_size) {
  if (file_size < min_doc_size_) {
    return min_doc_size_;
  } else if (file_size > max_doc_size_) {
    return max_doc_size_;
  } else {
    // Add 25% to document size
    size_t doc_size = file_size + (file_size >> 2);
    return min(doc_size, max_doc_size_);
  }
}

}  // namespace inamata