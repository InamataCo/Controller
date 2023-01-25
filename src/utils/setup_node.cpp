#include "setup_node.h"

#include "managers/web_socket.h"
#include "tasks/connectivity/connectivity.h"
#ifdef ESP32
#include "tasks/system_monitor/system_monitor.h"
#endif

namespace inamata {

bool loadNetwork(Services& services, JsonObjectConst secrets) {
  JsonArrayConst wifi_aps_doc = secrets[F("wifi_aps")];
  TRACEF("Found %u APs in secrets\n", wifi_aps_doc.size());
  
  std::vector<WiFiAP> wifi_aps;
  for (const JsonObjectConst i : wifi_aps_doc) {
    wifi_aps.emplace_back(WiFiAP{.ssid = i[F("ssid")].as<const char*>(),
                                 .password = i[F("password")].as<const char*>(),
                                 .id = -1,
                                 .failed_connecting = false});
  }
  String controller_name = secrets[F("name")].as<const char*>();
  services.setNetwork(std::make_shared<Network>(wifi_aps, controller_name));
  return true;
}

bool loadWebsocket(Services& services, JsonObjectConst secrets) {
  using namespace std::placeholders;

  // Get the required data from the secrets file
  JsonVariantConst ws_token = secrets[WebSocket::ws_token_key_];
  JsonVariantConst core_domain = secrets[WebSocket::core_domain_key_];
  JsonVariantConst secure_url = secrets[WebSocket::secure_url_key_];

  // Load the root certificate authority files for TLS encryption
  String root_cas;
  fs::File root_cas_file = LittleFS.open("/root_cas.pem", "r");
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
#ifdef ESP32
  OtaUpdater& ota_updater = services.getOtaUpdater();
#endif

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
#ifdef ESP32
      .ota_update_callback =
          std::bind(&OtaUpdater::handleCallback, &ota_updater, _1),
#endif
      .core_domain = core_domain.as<const char*>(),
      .ws_token = ws_token.as<const char*>(),
      .secure_url = secure_url};
  services.setWebSocket(std::make_shared<WebSocket>(config, std::move(root_cas)));
  return true;
}

bool createSystemTasks(Services& services) {
  // Temporary storage for the created system tasks
  std::vector<tasks::BaseTask*> tasks;

  // Create the system tasks
  tasks.push_back(new tasks::connectivity::CheckConnectivity(
      services.getGetters(), services.getScheduler()));
#ifdef ESP32
  tasks.push_back(new tasks::system_monitor::SystemMonitor(
      services.getGetters(), services.getScheduler()));
#endif

  // Check if they were created, enable them and check if they started
  for (tasks::BaseTask* task : tasks) {
    if (!task) {
      TRACELN(F("Failed creating task"));
      return false;
    }
    task->enable();
    if (!task->isValid()) {
      TRACELN(task->getError().toString());
      delete task;
      return false;
    }
  }
  return true;
}

bool setupNode(Services& services) {
  // Enable serial communication and prints
  Serial.begin(115200);
  Serial.print(F("Fimware version: "));
  Serial.println(WebSocket::firmware_version_);
  WiFi.begin();

  // Load and start subsystems that need secrets
  services.setStorage(std::make_shared<Storage>());
  JsonVariantConst secrets = services.getStorage()->openSecrets();
  bool success = loadNetwork(services, secrets);
  if (!success) {
    return false;
  }
  success = loadWebsocket(services, secrets);
  if (!success) {
    return false;
  }
  services.getStorage()->closeSecrets(false);

  success = createSystemTasks(services);
  if (!success) {
    return false;
  }
  return true;
}

}  // namespace inamata
