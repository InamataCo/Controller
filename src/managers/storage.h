#pragma once

#include <ArduinoJson.h>

#ifdef ESP32
#include <LittleFS.h>
#else
#include <EEPROM.h>
#include <StreamUtils.h>
#endif

#include "managers/logging.h"
#include "managers/types.h"

namespace inamata {

class Storage {
 public:
  DynamicJsonDocument& openSecrets();
  void closeSecrets(bool save = true);

 private:
  void saveSecrets();
  size_t fileToDocSize(size_t file_size);

  DynamicJsonDocument secrets_doc_ = DynamicJsonDocument(0);
#ifdef ESP32
  const char* secrets_path_ = "/secrets.json";
#endif
  static constexpr size_t min_doc_size_ = 1024;
  static constexpr size_t max_doc_size_ = 2048;
};
}  // namespace inamata
