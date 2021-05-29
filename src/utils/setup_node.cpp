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
  JsonVariantConst ws_token = secrets[F("ws_token")];
  if (!ws_token.is<const char*>()) {
    Serial.println(F("Empty/no ws_token in secrets"));
    return false;
  }
  JsonVariantConst core_domain = secrets[F("core_domain")];
  if (!core_domain.is<const char*>()) {
    Serial.println(F("Empty/no core_domain in secrets"));
    return false;
  }

  JsonVariantConst force_insecure = secrets["force_insecure"];
  String root_cas;
  if (!(force_insecure.is<bool>() && force_insecure == true)) {
    // Load the root certificate authority files for TLS encryption
    fs::File root_cas_file = SPIFFS.open("/root_cas.pem", FILE_READ);
    if (root_cas_file) {
      root_cas.reserve(root_cas_file.size());
      while (root_cas_file.available()) {
        root_cas += char(root_cas_file.read());
      }
    }
    root_cas_file.close();
  }
  // Get the peripheral and task controllers
  peripheral::PeripheralController& peripheral_controller =
      services.getPeripheralController();
  tasks::TaskController& task_controller = services.getTaskController();

  // Create a websocket instance as the server interface
  services.setServer(std::make_shared<WebSocket>(
      std::bind(&peripheral::PeripheralController::getPeripheralIDs,
                &peripheral_controller),
      std::bind(&peripheral::PeripheralController::handleCallback,
                &peripheral_controller, _1),
      std::bind(&tasks::TaskController::getTaskIDs, &task_controller),
      std::bind(&tasks::TaskController::handleCallback, &task_controller, _1),
      core_domain.as<const char*>(), ws_token.as<const char*>(), root_cas.c_str()));
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
  Serial.println(F("Setup started"));
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
