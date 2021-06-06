#include "setup_node.h"

#include <FS.h>
#include <SPIFFS.h>

#include "managers/web_socket.h"
#include "tasks/connectivity/connectivity.h"
#include "tasks/system_monitor/system_monitor.h"

namespace bernd_box {

bool loadNetwork(Services& services, JsonObjectConst secrets) {
  JsonArrayConst wifi_aps_doc = secrets[F("wifi_aps")];
  if (!wifi_aps_doc.size()) {
    Serial.println(F("No WiFi APs in secrets"));
    return false;
  }
  std::vector<WiFiAP> wifi_aps;
  for (const JsonObjectConst i : wifi_aps_doc) {
    wifi_aps.emplace_back(
        WiFiAP{.ssid = i[F("ssid")].as<const char*>(),
               .password = i[F("password")].as<const char*>()});
  }
  services.setNetwork(std::make_shared<Network>(wifi_aps));
  return true;
}

bool loadWebsocket(Services& services, JsonObjectConst secrets) {
  using namespace std::placeholders;

  // Get the required data from the secrets file
  const __FlashStringHelper* ws_token_key = F("ws_token");
  JsonVariantConst ws_token = secrets[ws_token_key];
  if (!ws_token.is<const char*>()) {
    Serial.println(ErrorStore::genMissingProperty(
        ws_token_key, ErrorStore::KeyType::kString));
    return false;
  }

  const __FlashStringHelper* core_domain_key = F("core_domain");
  JsonVariantConst core_domain = secrets[core_domain_key];
  if (!core_domain.is<const char*>()) {
    Serial.println(ErrorStore::genMissingProperty(
        core_domain_key, ErrorStore::KeyType::kString));
    return false;
  }

  const __FlashStringHelper* secure_url_key = F("secure_url");
  JsonVariantConst secure_url = secrets[secure_url_key];
  if (!secure_url.is<bool>()) {
    Serial.println(ErrorStore::genMissingProperty(
        secure_url_key, ErrorStore::KeyType::kString));
    return false;
  }

  // Load the root certificate authority files for TLS encryption
  String root_cas;
  fs::File root_cas_file = SPIFFS.open("/root_cas.pem", FILE_READ);
  if (root_cas_file) {
    root_cas.reserve(root_cas_file.size());
    while (root_cas_file.available()) {
      root_cas += char(root_cas_file.read());
    }
  }
  root_cas_file.close();

  // Get the peripheral and task controllers
  peripheral::PeripheralController& peripheral_controller =
      services.getPeripheralController();
  tasks::TaskController& task_controller = services.getTaskController();
  OtaUpdater& ota_updater = services.getOtaUpdater();

  // Create a websocket instance as the server interface
  WebSocket::Config config{
      .get_peripheral_ids =
          std::bind(&peripheral::PeripheralController::getPeripheralIDs,
                    &peripheral_controller),
      .peripheral_controller_callback =
          std::bind(&peripheral::PeripheralController::handleCallback,
                    &peripheral_controller, _1),
      .get_task_ids =
          std::bind(&tasks::TaskController::getTaskIDs, &task_controller),
      .task_controller_callback = std::bind(
          &tasks::TaskController::handleCallback, &task_controller, _1),
      .ota_update_callback =
          std::bind(&OtaUpdater::handleCallback, &ota_updater, _1),
      .core_domain = core_domain.as<const char*>(),
      .ws_token = ws_token.as<const char*>(),
      .secure_url = secure_url};
  services.setServer(std::make_shared<WebSocket>(config, std::move(root_cas)));
  return true;
}

bool createSystemTasks(Services& services) {
  // Temporary storage for the created system tasks
  std::vector<tasks::BaseTask*> tasks;

  // Create the system tasks
  tasks.push_back(new tasks::connectivity::CheckConnectivity(
      services.getGetters(), services.getScheduler()));
  tasks.push_back(new tasks::system_monitor::SystemMonitor(
      services.getGetters(), services.getScheduler()));

  // Check if they were created, enable them and check if they started
  for (tasks::BaseTask* task : tasks) {
    if (!task) {
      Serial.println(F("Failed creating task"));
      return false;
    }
    task->enable();
    if (!task->isValid()) {
      Serial.println(task->getError().toString());
      delete task;
      return false;
    }
  }
  return true;
}

bool setupNode(Services& services) {
  bool format_spiffs_if_failed = false;

  // Enable serial communication and prints
  Serial.begin(115200);
  Serial.print(F("Fimware version: "));
  Serial.println(WebSocket::firmware_version_);
  WiFi.begin();

  // Initialize the file system
  if (!SPIFFS.begin(format_spiffs_if_failed)) {
    Serial.println(F("Failed mounting SPIFFS"));
    return false;
  }

  // Load a common config file for the subsystems
  fs::File secrets_file = SPIFFS.open("/secrets.json", FILE_READ);
  if (!secrets_file) {
    Serial.println(F("Failed opening secrets.json"));
    return false;
  }
  DynamicJsonDocument secrets_doc(secrets_file.size() * 1.5);
  DeserializationError error = deserializeJson(secrets_doc, secrets_file);
  secrets_file.close();
  if (error) {
    Serial.println(F("Failed parsing secrets.json"));
    return false;
  }

  // Load and start subsystems
  bool success = loadNetwork(services, secrets_doc.as<JsonObjectConst>());
  if (!success) {
    return false;
  }
  success = loadWebsocket(services, secrets_doc.as<JsonObjectConst>());
  if (!success) {
    return false;
  }
  success = createSystemTasks(services);
  if (!success) {
    return false;
  }
  return true;
}

}  // namespace bernd_box
