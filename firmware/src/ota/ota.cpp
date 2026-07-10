#include "ota_handler.h"
#include "config.h"
#include "driver/gpio.h"
#include "esp_ota_ops.h"
#include "web_assets.h" // Được sinh tự động từ script python
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <Update.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <WiFi.h>

#define SYS_LED_PIN LED_SYS

// --- Định nghĩa trạng thái hệ thống ---
enum SystemState { STATE_AP_MODE, STATE_STA_CONNECTING, STATE_STA_CONNECTED };

// --- Biến toàn cục (giới hạn trong ota.cpp dùng anonymous namespace) ---
namespace {
SystemState currentState = STATE_AP_MODE;
WebServer server(OTA_PORT);
WebSocketsServer webSocket = WebSocketsServer(WS_PORT);
DNSServer dnsServer;
Preferences preferences;

// WiFi credentials
String wifi_ssid = "";
String wifi_pass = "";

// Timers & Trạng thái LED
unsigned long stateTimer = 0;
unsigned long ledTimer = 0;
bool ledState = false;
unsigned long connectionLostTimer = 0;
bool wifiLostFlag = false;
} // namespace

// --- Khởi tạo AP Mode ---
void startAPMode() {
  currentState = STATE_AP_MODE;
  WiFi.disconnect();
  delay(100);

  WiFi.mode(WIFI_AP);
  // AP mở không mật khẩu để dễ kết nối
  WiFi.softAP(AP_SSID);
  delay(100);

  // Khởi động DNS Server để chuyển hướng toàn bộ tên miền về IP ESP32 (Captive
  // Portal)
  dnsServer.start(53, "*", WiFi.softAPIP());

  Serial.println("\n[OTA] Da khoi dong AP Mode!");
  Serial.printf("[OTA] SSID: %s (Open)\n", AP_SSID);
  Serial.print("[OTA] IP AP: ");
  Serial.println(WiFi.softAPIP());

  // Khởi động lại MDNS
  MDNS.end();
  if (MDNS.begin("datn")) {
    MDNS.addService("http", "tcp", OTA_PORT);
  }
}

// --- Thử kết nối WiFi (STA Mode) ---
void startConnectingSTA() {
  currentState = STATE_STA_CONNECTING;
  stateTimer = millis(); // Bắt đầu đếm ngược 15 giây

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());

  Serial.print("\n[OTA] Dang ket noi toi WiFi: ");
  Serial.println(wifi_ssid);
}

// --- Đọc cấu hình WiFi từ bộ nhớ Flash ---
void loadCredentials() {
  preferences.begin("wifi-creds", true); // true = Read-Only
  wifi_ssid = preferences.getString("ssid", "");
  wifi_pass = preferences.getString("pass", "");
  preferences.end();
}

// --- Lưu cấu hình WiFi vào bộ nhớ Flash ---
void saveCredentials(String ssid, String pass) {
  preferences.begin("wifi-creds", false);
  preferences.putString("ssid", ssid);
  preferences.putString("pass", pass);
  preferences.end();
}

// ===================== WEB SERVER HANDLERS =====================

void handleRoot() { server.send_P(200, "text/html", INDEX_HTML); }

void handleCSS() { server.send_P(200, "text/css", STYLE_CSS); }

void handleJS() { server.send_P(200, "application/javascript", SCRIPT_JS); }

void handleWiFiScan() {
  int16_t scanResult = WiFi.scanComplete();

  if (scanResult == WIFI_SCAN_RUNNING) {
    // Trả về mã 202 (Accepted) báo hiệu đang quét, client sẽ gửi lại request
    // sau
    server.send(202, "application/json", "{\"status\":\"scanning\"}");
    return;
  }

  if (scanResult == WIFI_SCAN_FAILED || scanResult < 0) {
    // Nếu chưa bắt đầu quét hoặc quét bị lỗi, kích hoạt quét bất đồng bộ mới
    WiFi.scanDelete();
    Serial.println("[OTA] Bat dau quet WiFi bat dong bo...");
    WiFi.scanNetworks(true); // true = Async mode
    server.send(202, "application/json", "{\"status\":\"scanning\"}");
    return;
  }

  // Nếu quét đã hoàn tất (scanResult >= 0 là số lượng mạng tìm thấy)
  Serial.printf("[OTA] Da quet xong, tim thay %d mang WiFi.\n", scanResult);
  String json = "[";
  for (int i = 0; i < scanResult; i++) {
    if (i > 0)
      json += ",";
    json += "{\"ssid\":\"" + WiFi.SSID(i) +
            "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
  }
  json += "]";

  // Giải phóng bộ nhớ quét
  WiFi.scanDelete();
  server.send(200, "application/json", json);
}

void handleWiFiSave() {
  if (server.hasArg("ssid")) {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");

    Serial.print("[OTA] Nhan duoc thong tin WiFi moi. SSID: ");
    Serial.println(ssid);

    saveCredentials(ssid, pass);
    server.send(200, "text/plain", "OK");

    delay(1000);
    ESP.restart(); // Reset chip để khởi động lại và kết nối WiFi mới
  } else {
    server.send(400, "text/plain", "Missing SSID");
  }
}

void handleWiFiForget() {
  Serial.println("[OTA] Nhan yeu cau Quen WiFi. Xoa cau hinh va reboot...");
  preferences.begin("wifi-creds", false);
  preferences.clear();
  preferences.end();
  server.send(200, "text/plain", "OK");

  delay(1000);
  ESP.restart(); // Reset chip để phát lại AP mode
}

String getSystemStatusJSON() {
  String currentSSID = (currentState == STATE_STA_CONNECTED) ? WiFi.SSID() : "";
  String currentIP = ota_get_current_ip();
  String modeStr = (currentState == STATE_AP_MODE) ? "AP" : "STATION";
  long rssi = (currentState == STATE_STA_CONNECTED) ? WiFi.RSSI() : 0;

  // Lấy tên phân vùng Flash hiện tại đang chạy
  const esp_partition_t *running = esp_ota_get_running_partition();
  String partitionName = (running != NULL) ? String(running->label) : "Unknown";

  float luxVal = sensors_read_light();

  String json = "{";
  json += "\"version\":\"" + String(FW_VERSION) + "\",";
  json += "\"ssid\":\"" + currentSSID + "\",";
  json += "\"ip\":\"" + currentIP + "\",";
  json += "\"heap\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"uptime\":" + String(millis()) + ",";
  json += "\"rssi\":" + String(rssi) + ",";
  json += "\"wifimode\":\"" + modeStr + "\",";
  json += "\"partition\":\"" + partitionName + "\",";
  json += "\"flash_size\":" + String(ESP.getFlashChipSize()) + ",";
  json += "\"sketch_size\":" + String(ESP.getSketchSize()) + ",";
  json += "\"free_sketch\":" + String(ESP.getFreeSketchSpace()) + ",";
  json += "\"lux\":" + String(luxVal, 1);
  json += "}";
  return json;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload,
                    size_t length) {
  switch (type) {
  case WStype_DISCONNECTED:
    Serial.printf("[WS] Client #%u disconnected!\n", num);
    break;
  case WStype_CONNECTED: {
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[WS] Client #%u connected from %d.%d.%d.%d\n", num, ip[0],
                  ip[1], ip[2], ip[3]);
    // Gửi ngay trạng thái hệ thống cho client vừa kết nối
    String statusJson = getSystemStatusJSON();
    webSocket.sendTXT(num, statusJson);
    break;
  }
  case WStype_TEXT:
    Serial.printf("[WS] Nhan tu client #%u: %s\n", num, payload);
    break;
  default:
    break;
  }
}

void handleSystemStatus() {
  server.send(200, "application/json", getSystemStatusJSON());
}

void handleUpdateUpload() {
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("[OTA] Bat dau upload firmware: %s\n",
                  upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Serial.printf("[OTA] Cap nhat hoan tat! Kich thuoc: %u bytes\n",
                    upload.totalSize);
    } else {
      Update.printError(Serial);
    }
  }
}

void handleUpdateEnd() {
  if (Update.hasError()) {
    server.send(500, "text/plain", "OTA Failed!");
  } else {
    server.send(200, "text/plain", "OK");
    delay(1000);
    ESP.restart();
  }
}

// --- Khởi tạo hệ thống WiFi/OTA ---
void ota_init() {
  gpio_reset_pin((gpio_num_t)SYS_LED_PIN);
  gpio_set_direction((gpio_num_t)SYS_LED_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)SYS_LED_PIN, 0); // Tắt LED ban đầu

  loadCredentials();

  // Cấu hình các route cho WebServer
  server.on("/", HTTP_GET, handleRoot);
  server.on("/style.css", HTTP_GET, handleCSS);
  server.on("/script.js", HTTP_GET, handleJS);
  server.on("/api/wifi/scan", HTTP_GET, handleWiFiScan);
  server.on("/api/wifi/save", HTTP_POST, handleWiFiSave);
  server.on("/api/wifi/forget", HTTP_POST, handleWiFiForget);
  server.on("/api/system/status", HTTP_GET, handleSystemStatus);

  // Route xử lý upload firmware OTA
  server.on("/update", HTTP_POST, handleUpdateEnd, handleUpdateUpload);

  // Chuyển hướng Captive Portal (nếu truy cập tên miền lạ khi đang ở chế độ AP)
  server.onNotFound([]() {
    String host = server.hostHeader();
    if (host != "192.168.4.1" && host != "datn.local" && host != "datn") {
      server.sendHeader("Location", "http://192.168.4.1/", true);
      server.send(302, "text/plain", "");
    } else {
      server.send(404, "text/plain", "Not Found");
    }
  });

  // Bắt đầu khởi tạo WiFi trước để nạp LwIP stack
  if (wifi_ssid.length() > 0) {
    startConnectingSTA();
  } else {
    startAPMode();
  }

  // Sau khi WiFi stack (LwIP) đã sẵn sàng, ta mới bắt đầu chạy Web Server
  server.begin();
  Serial.println("[OTA] Web Server da san sang va bat dau lang nghe!");

  // Khởi động WebSocket Server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("[OTA] WebSocket Server da san sang tren cong 81!");
}

// --- Hàm xử lý liên tục trong vòng lặp loop() ---
void ota_handle() {
  // 1. Chạy DNS Server nếu ở chế độ AP Mode
  if (currentState == STATE_AP_MODE) {
    dnsServer.processNextRequest();
  }

  // 2. Chạy Web Server nhận request
  server.handleClient();

  // 3. Xử lý trạng thái LED nhấp nháy
  unsigned long now = millis();

  if (currentState == STATE_STA_CONNECTED) {
    // Đã kết nối WiFi -> LED sáng liên tục
    gpio_set_level((gpio_num_t)SYS_LED_PIN, 1);
  } else {
    // Đang kết nối hoặc ở chế độ AP -> LED nhấp nháy chu kỳ 1s (500ms sáng /
    // 500ms tắt)
    if (now - ledTimer >= 500) {
      ledTimer = now;
      ledState = !ledState;
      gpio_set_level((gpio_num_t)SYS_LED_PIN, ledState ? 1 : 0);
    }
  }

  // 4. Xử lý State Machine của WiFi
  if (currentState == STATE_STA_CONNECTING) {
    if (WiFi.status() == WL_CONNECTED) {
      currentState = STATE_STA_CONNECTED;
      Serial.println("\n[OTA] Ket noi WiFi thanh cong!");
      Serial.print("[OTA] IP STA: ");
      Serial.println(WiFi.localIP());

      // Khởi chạy MDNS để truy cập bằng tên miền: http://datn.local
      MDNS.end();
      if (MDNS.begin("datn")) {
        MDNS.addService("http", "tcp", OTA_PORT);
        Serial.println(
            "[OTA] MDNS da khoi dong! Truy cap qua: http://datn.local");
      }
    } else if (now - stateTimer >= 15000) {
      // Sau 15 giây không kết nối được WiFi -> Chuyển sang AP Mode
      Serial.println(
          "\n[OTA] Qua 15s khong ket noi duoc WiFi. Chuyen ve AP Mode...");
      startAPMode();
    }
  }

  // 5. Kiểm tra mất kết nối đột ngột ở chế độ STATION
  if (currentState == STATE_STA_CONNECTED) {
    if (WiFi.status() != WL_CONNECTED) {
      if (!wifiLostFlag) {
        wifiLostFlag = true;
        connectionLostTimer = now;
        Serial.println("\n[OTA] Mat ket noi WiFi. Cho 15s thu reconnect...");
      } else if (now - connectionLostTimer >= 15000) {
        // Mất kết nối liên tục quá 15 giây -> Fallback về AP Mode
        wifiLostFlag = false;
        Serial.println(
            "[OTA] Khong the ket noi lai WiFi sau 15s. Fallback ve AP Mode...");
        startAPMode();
      }
    } else {
      wifiLostFlag = false; // Đã kết nối lại bình thường
    }
  }

  // Xử lý WebSocket
  webSocket.loop();

  // Phát quảng bá trạng thái định kỳ 5 giây
  static unsigned long lastWsBroadcast = 0;
  if (now - lastWsBroadcast >= 5000) {
    lastWsBroadcast = now;
    String statusJson = getSystemStatusJSON();
    webSocket.broadcastTXT(statusJson);
  }
}

// --- Lấy IP hiện tại ---
String ota_get_current_ip() {
  if (currentState == STATE_AP_MODE) {
    return WiFi.softAPIP().toString();
  }
  return WiFi.localIP().toString();
}

// --- Kiểm tra kết nối ---
bool ota_is_sta_connected() { return (currentState == STATE_STA_CONNECTED); }
