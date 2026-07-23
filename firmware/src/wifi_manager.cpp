#include "wifi_manager.h"

#include "config.h"
#include "rtos_app.h"

#include <ESPmDNS.h>
#include <Preferences.h>

namespace {
DNSServer dns_server;   // Máy chủ DNS bẫy Captive Portal ở chế độ AP
Preferences preferences;// Đối tượng đọc/ghi bộ nhớ NVS Flash bền vững

NetworkState current_state = NetworkState::AP_MODE; // Trạng thái Wi-Fi mặc định ban đầu
String wifi_ssid; // Tên Wi-Fi nhà đã lưu
String wifi_pass; // Mật khẩu Wi-Fi nhà đã lưu
bool dns_running = false; // Chỉ xử lý DNS khi captive portal đã khởi động

// Hàm phụ ngắt toàn bộ ngõ ra khẩn cấp khi phát SoftAP
bool queueAllOff(const char *reason) {
  (void)reason;
  ControlCommand command{};
  command.type = ControlCommandType::ALL_OFF;
  command.created_at_ms = millis();
  command.ttl_ms = CONTROL_COMMAND_TTL_MS;
  return app_send_control_command(command, 0);
}
} // namespace

// ============================================================================
// KHỞI TẠO VÀ ĐỌC THÔNG TIN WI-FI TỪ NVS FLASH
// ============================================================================
void wifi_manager_init() {
  wifi_manager_load_credentials();
}

void wifi_manager_load_credentials() {
  preferences.begin("wifi-creds", true); // Mở namespace "wifi-creds" ở chế độ Read-only
  wifi_ssid = preferences.getString("ssid", "");
  wifi_pass = preferences.getString("pass", "");
  preferences.end();
}

void wifi_manager_save_credentials(const String &ssid, const String &pass) {
  preferences.begin("wifi-creds", false); // Mở namespace "wifi-creds" ở chế độ Read-Write
  preferences.putString("ssid", ssid);
  preferences.putString("pass", pass);
  preferences.end();
  wifi_ssid = ssid;
  wifi_pass = pass;
}

void wifi_manager_clear_credentials() {
  preferences.begin("wifi-creds", false);
  preferences.clear(); // Xóa sạch thông tin Wi-Fi đã lưu
  preferences.end();
  wifi_ssid = "";
  wifi_pass = "";
}

// ============================================================================
// BẮT ĐẦU PHÁT SOFTAP VÀ BẤY CAPTIVE PORTAL (192.168.4.1 / DATN.LOCAL)
// ============================================================================
bool wifi_manager_start_ap() {
  queueAllOff("WIFI_AP_MODE_START"); // Phát lệnh ngắt tất cả các tải an toàn
  dns_server.stop();
  dns_running = false;
  MDNS.end();

  if (!WiFi.mode(WIFI_AP)) {
    Serial.println("[WIFI] ERROR: Khong the chuyen sang AP mode.");
    return false;
  }
  if (!WiFi.softAP(AP_SSID)) {
    Serial.println("[WIFI] ERROR: Khong the khoi dong SoftAP.");
    return false;
  }
  delay(100);

  current_state = NetworkState::AP_MODE;
  dns_running = dns_server.start(53, "*", WiFi.softAPIP());
  if (!dns_running) {
    Serial.println("[WIFI] WARNING: DNS captive portal khong khoi dong.");
  }

  if (MDNS.begin("datn")) {
    MDNS.addService("http", "tcp", OTA_PORT); // Hỗ trợ gõ http://datn.local
  } else {
    Serial.println("[WIFI] WARNING: mDNS khong khoi dong.");
  }

  Serial.print("[WIFI] AP ready, IP: ");
  Serial.println(WiFi.softAPIP());
  return true;
}

// Bắt đầu kết nối vào mạng Wi-Fi nhà (STA Mode)
bool wifi_manager_start_connecting_sta() {
  if (wifi_ssid.isEmpty()) {
    Serial.println("[WIFI] ERROR: SSID rong, khong the khoi dong STA.");
    return false;
  }

  dns_server.stop();
  dns_running = false;
  MDNS.end();
  if (!WiFi.mode(WIFI_STA)) {
    Serial.println("[WIFI] ERROR: Khong the chuyen sang STA mode.");
    return false;
  }

  const wl_status_t begin_status =
      WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
  if (begin_status == WL_CONNECT_FAILED) {
    Serial.println("[WIFI] ERROR: Khong the bat dau ket noi STA.");
    return false;
  }

  current_state = NetworkState::STA_CONNECTING;
  Serial.print("[WIFI] Dang ket noi STA: ");
  Serial.println(wifi_ssid);
  return true;
}

// Xử lý gói tin DNS bẫy Captive Portal
void wifi_manager_process_dns() {
  if (current_state == NetworkState::AP_MODE && dns_running) {
    dns_server.processNextRequest();
  }
}

// Kiểm tra và cập nhật trạng thái kết nối Wi-Fi thực tế
NetworkState wifi_manager_get_state() {
  if (current_state == NetworkState::STA_CONNECTING && WiFi.status() == WL_CONNECTED) {
    current_state = NetworkState::STA_CONNECTED;
    MDNS.end();
    if (MDNS.begin("datn")) {
      MDNS.addService("http", "tcp", OTA_PORT);
    }
  } else if (current_state == NetworkState::STA_CONNECTED && WiFi.status() != WL_CONNECTED) {
    current_state = NetworkState::STA_CONNECTING; // Nếu rớt Wi-Fi, chuyển về STA_CONNECTING
  }
  return current_state;
}

String wifi_manager_get_current_ip() {
  const NetworkState state = wifi_manager_get_state();
  if (state == NetworkState::STA_CONNECTED) {
    return WiFi.localIP().toString(); // Trả về IP Wi-Fi nhà
  }
  return WiFi.softAPIP().toString(); // Trả về IP AP 192.168.4.1
}

bool wifi_manager_is_sta_connected() {
  return wifi_manager_get_state() == NetworkState::STA_CONNECTED;
}

String wifi_manager_get_ssid() {
  return wifi_ssid;
}

String wifi_manager_get_pass() {
  return wifi_pass;
}

const char *wifi_manager_state_to_text(NetworkState state) {
  switch (state) {
  case NetworkState::AP_MODE:
    return "AP";
  case NetworkState::STA_CONNECTING:
    return "CONNECTING";
  case NetworkState::STA_CONNECTED:
    return "STATION";
  }
  return "UNKNOWN";
}
