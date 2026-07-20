#include "ota_handler.h"
#include "config.h"
#include "thresholds.h"
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

// Bộ đệm lưu trữ giá trị cảm biến (tránh đọc I2C trực tiếp gây treo/lag khi tắt/bật relay)
float cached_lux = -1.0;
bool cached_bh1750_connected = false;
float cached_temp_air = -1.0;
bool cached_sht31_connected = false;
float cached_humidity = -1.0;
float cached_temp_water = -1.0;
bool cached_ds18b20_connected = false;
float cached_tds = -1.0;
bool cached_tds_connected = false;
float cached_ph = -1.0;
bool cached_ph_connected = false;
float cached_flow = 0.0;
bool cached_flow_connected = false;
float cached_lvl1 = 0.0;
float cached_lvl2 = 0.0;
float cached_lvl3 = 0.0;
float cached_lvl4 = 0.0;
bool cached_lvl_connected = false;


// Hàm escape ký tự đặc biệt trong JSON
String escapeJson(String input) {
  input.replace("\\", "\\\\");
  input.replace("\"", "\\\"");
  input.replace("\n", "\\n");
  input.replace("\r", "\\r");
  input.replace("\t", "\\t");
  return input;
}
} // namespace

// --- Khởi tạo AP Mode ---
void startAPMode() {
  currentState = STATE_AP_MODE;
  WiFi.disconnect();
  delay(100);

  WiFi.mode(WIFI_AP);
  // AP mở không mật khẩu để dễ kết nối
  if (!WiFi.softAP(AP_SSID)) {
    Serial.println("[OTA] LOI: Khong the khoi dong AP Mode!");
    return;
  }
  delay(100);

  // Khởi động DNS Server để chuyển hướng toàn bộ tên miền về IP ESP32 (Captive Portal)
  if (!dnsServer.start(53, "*", WiFi.softAPIP())) {
    Serial.println("[OTA] CANH BAO: DNS Server khong khoi dong duoc (Captive Portal se khong hoat dong)");
  }

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

  if (!WiFi.mode(WIFI_STA)) {
    Serial.println("[OTA] LOI: Khong the chuyen sang STA Mode!");
    startAPMode(); // Fallback về AP nếu không chuyển được
    return;
  }

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

void handleRoot() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Content-Encoding", "gzip");
  server.send_P(200, "text/html", (const char*)INDEX_HTML_GZ, INDEX_HTML_GZ_LEN);
}

void handleCSS() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Content-Encoding", "gzip");
  server.send_P(200, "text/css", (const char*)STYLE_CSS_GZ, STYLE_CSS_GZ_LEN);
}

void handleJS() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Content-Encoding", "gzip");
  server.send_P(200, "application/javascript", (const char*)SCRIPT_JS_GZ, SCRIPT_JS_GZ_LEN);
}

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

  // Giới hạn tối đa 20 mạng để tránh tràn bộ nhớ (vùng đông dân có thể có 100+ mạng)
  const int MAX_NETWORKS = 20;
  int displayCount = (scanResult > MAX_NETWORKS) ? MAX_NETWORKS : scanResult;

  String json = "[";
  for (int i = 0; i < displayCount; i++) {
    if (i > 0)
      json += ",";
    json += "{\"ssid\":\"" + escapeJson(WiFi.SSID(i)) +
            "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
  }
  json += "]";

  // Giải phóng bộ nhớ quét
  WiFi.scanDelete();

  if (scanResult > MAX_NETWORKS) {
    Serial.printf("[OTA] Chi hien thi %d/%d mang WiFi de tranh qua tai bo nho.\n", MAX_NETWORKS, scanResult);
  }

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

    // Force flush trước khi restart
    actuator_force_flush();

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

  // Force flush trước khi restart
  actuator_force_flush();

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

  String escapedSSID = escapeJson(currentSSID);

  // Tăng buffer lên 2048 bytes để an toàn với SSID dài + escape sequences
  static char buf[2048];
  snprintf(buf, sizeof(buf),
    "{"
    "\"version\":\"%s\","
    "\"ssid\":\"%s\","
    "\"ip\":\"%s\","
    "\"heap\":%u,"
    "\"uptime\":%lu,"
    "\"rssi\":%ld,"
    "\"wifimode\":\"%s\","
    "\"partition\":\"%s\","
    "\"flash_size\":%u,"
    "\"sketch_size\":%u,"
    "\"free_sketch\":%u,"
    "\"lux\":%.1f,"
    "\"lux_conn\":%d,"
    "\"temp\":%.1f,"
    "\"temp_conn\":%d,"
    "\"humi\":%.1f,"
    "\"humi_conn\":%d,"
    "\"temp_w\":%.1f,"
    "\"temp_w_conn\":%d,"
    "\"tds\":%.1f,"
    "\"tds_conn\":%d,"
    "\"ph\":%.1f,"
    "\"ph_conn\":%d,"
    "\"flow\":%.1f,"
    "\"flow_conn\":%d,"
    "\"lvl1\":%.1f,"
    "\"lvl1_conn\":%d,"
    "\"lvl2\":%.1f,"
    "\"lvl2_conn\":%d,"
    "\"lvl3\":%.1f,"
    "\"lvl3_conn\":%d,"
    "\"lvl4\":%.1f,"
    "\"lvl4_conn\":%d,"
    "\"act_IN_RL1\":%d,"
    "\"act_IN_RL2\":%d,"
    "\"act_DEN1\":%d,"
    "\"act_DEN2\":%d,"
    "\"act_QUAT1\":%d,"
    "\"act_QUAT2\":%d,"
    "\"act_BOMLL1\":%d,"
    "\"act_BOMLL2\":%d,"
    "\"act_BOMLL3\":%d,"
    "\"act_BOM12V\":%d,"
    "\"th_light_l\":%.1f,"
    "\"th_light_h\":%.1f,"
    "\"th_tempw_l\":%.1f,"
    "\"th_tempw_h\":%.1f,"
    "\"th_tempa_l\":%.1f,"
    "\"th_tempa_h\":%.1f,"
    "\"th_humi_l\":%.1f,"
    "\"th_humi_h\":%.1f,"
    "\"th_tds_l\":%.1f,"
    "\"th_tds_h\":%.1f,"
    "\"th_ph_l\":%.2f,"
    "\"th_ph_h\":%.2f"
    "}",
    FW_VERSION,
    escapedSSID.c_str(),
    currentIP.c_str(),
    ESP.getFreeHeap(),
    millis(),
    rssi,
    modeStr.c_str(),
    partitionName.c_str(),
    ESP.getFlashChipSize(),
    ESP.getSketchSize(),
    ESP.getFreeSketchSpace(),
    cached_lux,
    cached_bh1750_connected ? 1 : 0,
    cached_temp_air,
    cached_sht31_connected ? 1 : 0,
    cached_humidity,
    cached_sht31_connected ? 1 : 0,
    cached_temp_water,
    cached_ds18b20_connected ? 1 : 0,
    cached_tds,
    cached_tds_connected ? 1 : 0,
    cached_ph,
    cached_ph_connected ? 1 : 0,
    cached_flow,
    cached_flow_connected ? 1 : 0,
    cached_lvl1,
    cached_lvl_connected ? 1 : 0,
    cached_lvl2,
    cached_lvl_connected ? 1 : 0,
    cached_lvl3,
    cached_lvl_connected ? 1 : 0,
    cached_lvl4,
    cached_lvl_connected ? 1 : 0,
    actuator_get_state(IN_RL1),
    actuator_get_state(IN_RL2),
    actuator_get_state(DEN1),
    actuator_get_state(DEN2),
    actuator_get_state(QUAT1),
    actuator_get_state(QUAT2),
    actuator_get_state(BOMLL1),
    actuator_get_state(BOMLL2),
    actuator_get_state(BOMLL3),
    actuator_get_state(BOM12V),
    (float)THRESHOLD_LIGHT_LOW,
    (float)THRESHOLD_LIGHT_HIGH,
    (float)THRESHOLD_TEMP_WATER_LOW,
    (float)THRESHOLD_TEMP_WATER_HIGH,
    (float)THRESHOLD_TEMP_AIR_LOW,
    (float)THRESHOLD_TEMP_AIR_HIGH,
    (float)THRESHOLD_HUMIDITY_LOW,
    (float)THRESHOLD_HUMIDITY_HIGH,
    (float)THRESHOLD_TDS_LOW,
    (float)THRESHOLD_TDS_HIGH,
    (float)THRESHOLD_PH_LOW,
    (float)THRESHOLD_PH_HIGH
  );
  return String(buf);
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
  case WStype_TEXT: {
    Serial.printf("[WS] Nhan tu client #%u: %s\n", num, payload);
    String msg = (char*)payload;
    int pin = -1;
    int state = -1;

    // Parse JSON thủ công đơn giản (tối ưu cho embedded, không cần thư viện nặng)
    // Format mong đợi: {"pin":7,"state":1} hoặc {"pin": 7, "state": 1}
    int pinIdx = msg.indexOf("\"pin\"");
    int stateIdx = msg.indexOf("\"state\"");

    if (pinIdx != -1 && stateIdx != -1) {
      // Tìm vị trí dấu hai chấm sau "pin"
      int colonPin = msg.indexOf(":", pinIdx);
      if (colonPin != -1) {
        // Tìm dấu phẩy hoặc dấu ngoặc đóng
        int commaPin = msg.indexOf(",", colonPin);
        if (commaPin == -1) commaPin = msg.indexOf("}", colonPin);
        if (commaPin != -1) {
          String pinStr = msg.substring(colonPin + 1, commaPin);
          pinStr.trim(); // Loại bỏ khoảng trắng
          pin = pinStr.toInt();
        }
      }

      // Tìm vị trí dấu hai chấm sau "state"
      int colonState = msg.indexOf(":", stateIdx);
      if (colonState != -1) {
        int commaState = msg.indexOf(",", colonState);
        if (commaState == -1) commaState = msg.indexOf("}", colonState);
        if (commaState != -1) {
          String stateStr = msg.substring(colonState + 1, commaState);
          stateStr.trim(); // Loại bỏ khoảng trắng
          state = stateStr.toInt();
        }
      }
    }

    // Validate pin number trước khi gọi actuator
    const int valid_pins[] = {IN_RL1, IN_RL2, DEN1, DEN2, QUAT1, QUAT2,
                              BOMLL1, BOMLL2, BOMLL3, BOM12V};
    bool pin_valid = false;
    if (pin != -1) {
      for (int vp : valid_pins) {
        if (pin == vp) {
          pin_valid = true;
          break;
        }
      }
    }

    if (pin_valid && state != -1) {
      Serial.printf("[WS] Dieu khien thiet bi: GPIO %d -> Gtri %d\n", pin, state);
      actuator_set_state(pin, state);

      // Phát thông tin cập nhật trạng thái mới cho toàn bộ các thiết bị đang kết nối
      String statusJson = getSystemStatusJSON();
      webSocket.broadcastTXT(statusJson);
    } else {
      Serial.printf("[WS] LOI: JSON khong hop le hoac pin khong hop le. pin=%d, state=%d\n", pin, state);
      // Gửi thông báo lỗi về client
      String errorMsg = "{\"error\":\"Invalid pin or state\"}";
      webSocket.sendTXT(num, errorMsg);
    }
    break;
  }
  default:
    break;
  }
}

void handleSystemStatus() {
  server.send(200, "application/json", getSystemStatusJSON());
}

void handleControl() {
  if (server.hasArg("pin") && server.hasArg("state")) {
    int pin = server.arg("pin").toInt();
    int state = server.arg("state").toInt();

    // Validate pin number trước khi gọi actuator (giống WebSocket handler)
    const int valid_pins[] = {IN_RL1, IN_RL2, DEN1, DEN2, QUAT1, QUAT2,
                              BOMLL1, BOMLL2, BOMLL3, BOM12V};
    bool pin_valid = false;
    for (int vp : valid_pins) {
      if (pin == vp) {
        pin_valid = true;
        break;
      }
    }

    if (!pin_valid) {
      Serial.printf("[HTTP-CTRL] LOI: Pin khong hop le: GPIO %d\n", pin);
      server.send(400, "application/json", "{\"error\":\"Invalid pin\"}");
      return;
    }

    Serial.printf("[HTTP-CTRL] Dieu khien thiet bi: GPIO %d -> Gtri %d\n", pin, state);
    actuator_set_state(pin, state);

    // Phát trạng thái mới qua WebSocket cho tất cả các client đang online cập nhật
    String statusJson = getSystemStatusJSON();
    webSocket.broadcastTXT(statusJson);

    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(400, "text/plain", "Missing pin or state");
  }
}

void handleUpdateUpload() {
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("[OTA] Bat dau upload firmware: %s\n",
                  upload.filename.c_str());

    // Force flush tất cả giá trị PWM pending trước khi OTA
    actuator_force_flush();

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
  loadCredentials();

  // Cấu hình các route cho WebServer
  server.on("/", HTTP_GET, handleRoot);
  server.on("/style.css", HTTP_GET, handleCSS);
  server.on("/script.js", HTTP_GET, handleJS);
  server.on("/api/wifi/scan", HTTP_GET, handleWiFiScan);
  server.on("/api/wifi/save", HTTP_POST, handleWiFiSave);
  server.on("/api/wifi/forget", HTTP_POST, handleWiFiForget);
  server.on("/api/system/status", HTTP_GET, handleSystemStatus);
  server.on("/api/control", HTTP_POST, handleControl);

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

  // Cập nhật giá trị cảm biến ban đầu vào cache
  cached_lux = sensors_read_light();
  cached_bh1750_connected = sensors_is_bh1750_connected();
  cached_temp_air = sensors_read_temp_air();
  cached_sht31_connected = sensors_is_sht31_connected();
  cached_humidity = sensors_read_humidity();
  cached_temp_water = sensors_read_temp_water();
  cached_ds18b20_connected = sensors_is_ds18b20_connected();
  cached_tds = sensors_read_tds(cached_temp_water);
  cached_tds_connected = sensors_is_tds_connected();
  cached_ph = sensors_read_ph();
  cached_ph_connected = sensors_is_ph_connected();
  cached_flow = sensors_read_flow();
  cached_flow_connected = sensors_is_flow_connected();
  cached_lvl1 = sensors_read_level1();
  cached_lvl2 = sensors_read_level2();
  cached_lvl3 = sensors_read_level3();
  cached_lvl4 = sensors_read_level4();
  cached_lvl_connected = true;
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
    digitalWrite(SYS_LED_PIN, HIGH);
  } else {
    // Đang kết nối hoặc ở chế độ AP -> LED nhấp nháy chu kỳ 1s (500ms sáng /
    // 500ms tắt)
    if (now - ledTimer >= 500) {
      ledTimer = now;
      ledState = !ledState;
      digitalWrite(SYS_LED_PIN, ledState ? HIGH : LOW);
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

  // Phát quảng bá trạng thái định kỳ 1 giây
  static unsigned long lastWsBroadcast = 0;
  if (now - lastWsBroadcast >= 1000) {
    lastWsBroadcast = now;
    // Cập nhật bộ đệm cảm biến định kỳ
    cached_lux = sensors_read_light();
    cached_bh1750_connected = sensors_is_bh1750_connected();
    cached_temp_air = sensors_read_temp_air();
    cached_sht31_connected = sensors_is_sht31_connected();
    cached_humidity = sensors_read_humidity();
    cached_temp_water = sensors_read_temp_water();
    cached_ds18b20_connected = sensors_is_ds18b20_connected();
    cached_tds = sensors_read_tds(cached_temp_water);
    cached_tds_connected = sensors_is_tds_connected();
    cached_ph = sensors_read_ph();
    cached_ph_connected = sensors_is_ph_connected();
    cached_flow = sensors_read_flow();
    cached_flow_connected = sensors_is_flow_connected();
    cached_lvl1 = sensors_read_level1();
    cached_lvl2 = sensors_read_level2();
    cached_lvl3 = sensors_read_level3();
    cached_lvl4 = sensors_read_level4();
    cached_lvl_connected = true;
    
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
