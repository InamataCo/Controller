#include "connectivity.h"

#include "configuration.h"

namespace inamata {
namespace tasks {
namespace connectivity {

CheckConnectivity::CheckConnectivity(const ServiceGetters& services,
                                     Scheduler& scheduler)
    : BaseTask(scheduler), services_(services) {
  Task::setIterations(TASK_FOREVER);
  Task::setInterval(
      std::chrono::milliseconds(check_connectivity_period).count());
}

CheckConnectivity::~CheckConnectivity() {}

const String& CheckConnectivity::getType() const { return type(); }

const String& CheckConnectivity::type() {
  static const String name{"CheckConnectivity"};
  return name;
}

bool CheckConnectivity::OnTaskEnable() {
  network_ = services_.getNetwork();
  if (network_ == nullptr) {
    setInvalid(services_.network_nullptr_error_);
    return false;
  }
  web_socket_ = services_.getWebSocket();
  if (web_socket_ == nullptr) {
    setInvalid(services_.web_socket_nullptr_error_);
    return false;
  }

  return Callback();
}

bool CheckConnectivity::TaskCallback() {
  if (mode_ == Mode::ConnectWiFi) {
    Network::ConnectMode connect_mode = network_->connect();
    if (connect_mode == Network::ConnectMode::kCyclePower) {
      setMode(Mode::RunCaptivePortal);
    }
    if (connect_mode == Network::ConnectMode::kConnected) {
      checkInternetTime();
      handleWebSocket();
    }
  } else {
    if (std::chrono::steady_clock::now() - mode_start_ >
        std::chrono::minutes(3)) {
      if (disable_captive_portal_timeout_) {
        TRACELN(F("Captive portal timeout reset"));
        mode_start_ = std::chrono::steady_clock::now();
      } else {
        TRACELN(F("Captive portal timed out"));
        setMode(Mode::ConnectWiFi);
      }
    }
    handleCaptivePortal();
  }

  return true;
}

bool CheckConnectivity::checkInternetTime() {
  if (utils::chrono_abs(std::chrono::steady_clock::now() - last_time_check_) >
      time_check_period_) {
    last_time_check_ = std::chrono::steady_clock::now();

    int error = network_->setClock(std::chrono::seconds(30));
    if (error) {
      TRACELN(F("Failed to update time.Restarting in 10s"));
      delay(10000);
      ESP.restart();
    }
  }
  return true;
}

void CheckConnectivity::handleWebSocket() {
  WebSocket::ConnectState state = web_socket_->handle();
  if (state == WebSocket::ConnectState::kFailed) {
    TRACELN(F("WS failed"));
    setMode(Mode::RunCaptivePortal);
  }
}

void CheckConnectivity::handleCaptivePortal() {
  if (!wifi_manager_) {
    setupCaptivePortal();
  }
  bool connected = wifi_manager_->process();
  if (connected) {
    TRACELN(F("Connected!"));
    setMode(Mode::ConnectWiFi);
  }
}

void CheckConnectivity::setupCaptivePortal() {
  TRACELN(F("Setting up captive portal"));
  wifi_manager_ = std::unique_ptr<WiFiManager>(new WiFiManager());
  wifi_manager_->setConfigPortalBlocking(false);
  wifi_manager_->setSaveConnectTimeout(
      std::chrono::duration_cast<std::chrono::seconds>(wifi_connect_timeout)
          .count());
  wifi_manager_->setSaveConfigCallback(
      std::bind(&CheckConnectivity::saveCaptivePortalWifi, this));
  wifi_manager_->setSaveParamsCallback(
      std::bind(&CheckConnectivity::saveCaptivePortalParameters, this));
  wifi_manager_->setPreOtaUpdateCallback(
      std::bind(&CheckConnectivity::preOtaUpdateCallback, this));

  // Add WebSocket token param. If set, use placeholder to avoid secret leakage
  String ws_token_state =
      web_socket_->isWsTokenSet() ? ws_token_placeholder_ : F("");
  ws_token_parameter_ =
      std::unique_ptr<WiFiManagerParameter>(new WiFiManagerParameter(
          WebSocket::ws_token_key_, WebSocket::ws_token_key_,
          ws_token_state.c_str(), 64));
  wifi_manager_->addParameter(ws_token_parameter_.get());

  // Add core domain param. Set default or config loaded from secrets.json
  core_domain_parameter_ =
      std::unique_ptr<WiFiManagerParameter>(new WiFiManagerParameter(
          WebSocket::core_domain_key_, WebSocket::core_domain_key_,
          web_socket_->core_domain_.c_str(), 32));
  wifi_manager_->addParameter(core_domain_parameter_.get());

  // Add boolean secure url param. "y" is true, "n" is false
  const char* secure_url_state = web_socket_->secure_url_ ? "y" : "n";
  secure_url_parameter_ = std::unique_ptr<WiFiManagerParameter>(
      new WiFiManagerParameter(WebSocket::secure_url_key_,
                               "Use TLS? y=yes, n=no", secure_url_state, 1));
  wifi_manager_->addParameter(secure_url_parameter_.get());

  String ssid(wifi_portal_ssid);
  String password(wifi_portal_password);
  wifi_manager_->startConfigPortal(ssid.c_str(), password.c_str());
}

void CheckConnectivity::saveCaptivePortalWifi() {
  TRACELN(F("Save WiFi"));

  // Save the WiFi credentials
  String ssid = wifi_manager_->getWiFiSSID(false);
  String wifi_password = wifi_manager_->getWiFiPass(false);
  TRACEF("Connected to %s:%s\n", ssid.c_str(), wifi_password.c_str());
  bool saved = false;
  for (WiFiAP& wifi_ap : network_->wifi_aps_) {
    if (ssid == wifi_ap.ssid) {
      wifi_ap.password = wifi_password;
      saved = true;
      break;
    }
  }
  if (!saved) {
    network_->wifi_aps_.push_back({.ssid = ssid, .password = wifi_password});
  }
  saved = false;

  DynamicJsonDocument& secrets = services_.getStorage()->openSecrets();
  JsonArray wifi_aps = secrets[F("wifi_aps")].to<JsonArray>();
  for (JsonObject wifi_ap : wifi_aps) {
    if (wifi_ap[F("ssid")] == ssid) {
      wifi_ap[F("password")] = wifi_password;
      saved = true;
      break;
    }
  }
  if (!saved) {
    JsonObject wifi_ap = wifi_aps.createNestedObject();
    wifi_ap[F("ssid")] = ssid;
    wifi_ap[F("password")] = wifi_password;
  }
  services_.getStorage()->closeSecrets();
}

void CheckConnectivity::saveCaptivePortalParameters() {
  TRACELN(F("Save parameters"));
  DynamicJsonDocument& secrets = services_.getStorage()->openSecrets();

  // Save the other parameters
  WiFiManagerParameter** parameters = wifi_manager_->getParameters();
  for (int i = 0; i < wifi_manager_->getParametersCount(); i++) {
    WiFiManagerParameter* param = parameters[i];
    if (strcmp(param->getID(), WebSocket::ws_token_key_) == 0) {
      if (String(ws_token_placeholder_) != param->getValue()) {
        secrets[WebSocket::ws_token_key_] = param->getValue();
        web_socket_->setWsToken(param->getValue());
      }
    } else if (strcmp(param->getID(), WebSocket::core_domain_key_) == 0) {
      secrets[WebSocket::core_domain_key_] = param->getValue();
    } else if (strcmp(param->getID(), WebSocket::secure_url_key_) == 0) {
      const char secure_url = *(param->getValue());
      if (secure_url == 'y' || secure_url == 'Y') {
        secrets[WebSocket::secure_url_key_] = true;
      } else if (secure_url == 'n' || secure_url == 'N') {
        secrets[WebSocket::secure_url_key_] = false;
      }
    }
  }

  // Save the input parameters to FS or EEPROM
  services_.getStorage()->closeSecrets();
}

void CheckConnectivity::preOtaUpdateCallback() {
  disable_captive_portal_timeout_ = true;
}

void CheckConnectivity::setMode(Mode mode) {
  // No actions if not changing mode
  if (mode == mode_) {
    return;
  }

  // Actions to run when leaving mode
  if (mode_ == Mode::RunCaptivePortal) {
#ifdef ESP32
    wifi_manager_ = nullptr;
    ws_token_parameter_ = nullptr;
    core_domain_parameter_ = nullptr;
    secure_url_parameter_ = nullptr;
#else
    std::chrono::steady_clock::time_point start_time =
        std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start_time <
           std::chrono::seconds(2)) {
      wifi_manager_->process();
    }
    TRACELN(F("Restart to close WifiManager"));
    ESP.restart();
#endif
  }

  mode_ = mode;
  mode_start_ = std::chrono::steady_clock::now();

  // Actions to run when entering mode
  if (mode_ == Mode::ConnectWiFi) {
    network_->setMode(Network::ConnectMode::kFastConnect);
  }
  if (mode == Mode::RunCaptivePortal) {
    disable_captive_portal_timeout_ = false;
  }
}

const __FlashStringHelper* CheckConnectivity::ws_token_placeholder_ =
    FPSTR("<token set>");

}  // namespace connectivity
}  // namespace tasks
}  // namespace inamata
