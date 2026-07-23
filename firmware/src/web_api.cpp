#include "web_api.h"

#include "actuator.h"
#include "config.h"
#include "ota_update.h"
#include "rtos_app.h"
#include "system_types.h"
#include "thresholds.h"
#include "web_assets.h"
#include "wifi_manager.h"

#include <esp_ota_ops.h>

namespace {
// Hàm Helper escape các ký tự đặc biệt trong chuỗi để tránh làm hỏng định dạng JSON
String escapeJson(String input) {
  input.replace("\\", "\\\\");
  input.replace("\"", "\\\"");
  input.replace("\n", "\\n");
  input.replace("\r", "\\r");
  input.replace("\t", "\\t");
  return input;
}

// Chuyển Enum SystemMode sang chuỗi chữ
const char *modeToText(SystemMode mode) {
  switch (mode) {
  case SystemMode::MANUAL:
    return "MANUAL";
  case SystemMode::AUTO:
    return "AUTO";
  }
  return "UNKNOWN";
}

// Chuyển Enum SensorQuality sang chuỗi chữ
const char *qualityToText(SensorQuality quality) {
  switch (quality) {
  case SensorQuality::GOOD:
    return "GOOD";
  case SensorQuality::STALE:
    return "STALE";
  case SensorQuality::ERROR:
    return "ERROR";
  case SensorQuality::UNSTABLE:
    return "UNSTABLE";
  }
  return "UNKNOWN";
}

// Tính tuổi của dữ liệu cảm biến (tính bằng miligiây kể từ khi lấy mẫu)
uint32_t readingAge(uint32_t sampled_at_ms) {
  const uint32_t now = millis();
  return sampled_at_ms == 0 ? 0 : (now - sampled_at_ms);
}

// Trích xuất thông số của 1 cảm biến từ SensorSnapshot
SensorReading sensorById(const SensorSnapshot &snapshot, SensorId id) {
  return snapshot.readings[static_cast<size_t>(id)];
}

// Gửi lệnh tắt toàn bộ ngõ ra
bool queueAllOff(const char *reason) {
  ControlCommand command{};
  command.type = ControlCommandType::ALL_OFF;
  command.created_at_ms = millis();
  command.ttl_ms = CONTROL_COMMAND_TTL_MS;
  return app_send_control_command(command, 0);
}

// Gửi lệnh nhịp tim gia hạn Lease
bool queueHeartbeat() {
  ControlCommand command{};
  command.type = ControlCommandType::HEARTBEAT;
  command.created_at_ms = millis();
  command.ttl_ms = CONTROL_COMMAND_TTL_MS;
  return app_send_control_command(command, 0);
}
} // namespace

// ============================================================================
// HÀM DỰNG CHUỖI JSON TRẠNG THÁI HỆ THỐNG (SYSTEM STATUS JSON)
// ============================================================================
String web_api_build_status_json() {
  SensorSnapshot sensors{};
  ActuatorSnapshot outputs{};
  RuntimeHealth health{};
  // Lấy dữ liệu an toàn từ RAM (dùng khóa Mutex)
  app_get_sensor_snapshot(sensors);
  app_get_actuator_snapshot(outputs);
  app_get_runtime_health(health);

  const SensorReading temp_air = sensorById(sensors, SensorId::TEMP_AIR);
  const SensorReading humidity = sensorById(sensors, SensorId::HUMIDITY);
  const SensorReading lux = sensorById(sensors, SensorId::LIGHT);
  const SensorReading temp_water = sensorById(sensors, SensorId::TEMP_WATER);
  const SensorReading tds = sensorById(sensors, SensorId::TDS);
  const SensorReading ph = sensorById(sensors, SensorId::PH);
  const SensorReading flow = sensorById(sensors, SensorId::FLOW);
  const SensorReading lvl1 = sensorById(sensors, SensorId::LEVEL_MAIN);
  const SensorReading lvl2 = sensorById(sensors, SensorId::LEVEL_A);
  const SensorReading lvl3 = sensorById(sensors, SensorId::LEVEL_B);
  const SensorReading lvl4 = sensorById(sensors, SensorId::LEVEL_PH);

  const NetworkState state = wifi_manager_get_state();
  const String current_ssid = state == NetworkState::STA_CONNECTED ? WiFi.SSID() : "";
  const String current_ip = wifi_manager_get_current_ip();
  const long rssi = state == NetworkState::STA_CONNECTED ? WiFi.RSSI() : 0;
  const esp_partition_t *running = esp_ota_get_running_partition();
  const String partition_name = running != nullptr ? String(running->label) : "unknown";

  static char buffer[4096];
  snprintf(
      buffer, sizeof(buffer),
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
      "\"mode\":\"%s\","
      "\"all_off\":%d,"
      "\"lease_active\":%d,"
      "\"last_reason\":\"%s\","
      "\"ota_error\":\"%s\","
      "\"queue_cmd\":%u,"
      "\"queue_sensor\":%u,"
      "\"net_core\":%d,"
      "\"sensor_core\":%d,"
      "\"control_core\":%d,"
      "\"lux\":%.1f,"
      "\"lux_conn\":%d,"
      "\"lux_quality\":\"%s\","
      "\"lux_age\":%lu,"
      "\"temp\":%.1f,"
      "\"temp_conn\":%d,"
      "\"temp_quality\":\"%s\","
      "\"temp_age\":%lu,"
      "\"humi\":%.1f,"
      "\"humi_conn\":%d,"
      "\"humi_quality\":\"%s\","
      "\"humi_age\":%lu,"
      "\"temp_w\":%.1f,"
      "\"temp_w_conn\":%d,"
      "\"temp_w_quality\":\"%s\","
      "\"temp_w_age\":%lu,"
      "\"tds\":%.1f,"
      "\"tds_raw\":%u,"
      "\"tds_conn\":%d,"
      "\"tds_quality\":\"%s\","
      "\"tds_age\":%lu,"
      "\"ph\":%.2f,"
      "\"ph_conn\":%d,"
      "\"ph_quality\":\"%s\","
      "\"ph_age\":%lu,"
      "\"flow\":%.2f,"
      "\"flow_conn\":%d,"
      "\"flow_quality\":\"%s\","
      "\"flow_age\":%lu,"
      "\"lvl1\":%.1f,"
      "\"lvl1_conn\":%d,"
      "\"lvl1_quality\":\"%s\","
      "\"lvl1_age\":%lu,"
      "\"lvl2\":%.1f,"
      "\"lvl2_conn\":%d,"
      "\"lvl2_quality\":\"%s\","
      "\"lvl2_age\":%lu,"
      "\"lvl3\":%.1f,"
      "\"lvl3_conn\":%d,"
      "\"lvl3_quality\":\"%s\","
      "\"lvl3_age\":%lu,"
      "\"lvl4\":%.1f,"
      "\"lvl4_conn\":%d,"
      "\"lvl4_quality\":\"%s\","
      "\"lvl4_age\":%lu,"
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
      FW_VERSION, escapeJson(current_ssid).c_str(), current_ip.c_str(),
      ESP.getFreeHeap(), millis(), rssi, wifi_manager_state_to_text(state),
      partition_name.c_str(), ESP.getFlashChipSize(), ESP.getSketchSize(),
      ESP.getFreeSketchSpace(), modeToText(outputs.mode), outputs.all_off ? 1 : 0,
      outputs.web_lease_active ? 1 : 0, outputs.last_reason,
      escapeJson(ota_update_get_error_text()).c_str(),
      static_cast<unsigned>(app_command_queue_depth()),
      static_cast<unsigned>(app_sensor_queue_depth()), health.network_core,
      health.sensors_core, health.control_core, lux.value, lux.connected ? 1 : 0,
      qualityToText(lux.quality), readingAge(lux.sampled_at_ms), temp_air.value,
      temp_air.connected ? 1 : 0, qualityToText(temp_air.quality),
      readingAge(temp_air.sampled_at_ms), humidity.value,
      humidity.connected ? 1 : 0, qualityToText(humidity.quality),
      readingAge(humidity.sampled_at_ms), temp_water.value,
      temp_water.connected ? 1 : 0, qualityToText(temp_water.quality),
      readingAge(temp_water.sampled_at_ms), tds.value,
      static_cast<unsigned>(tds.raw_value), tds.connected ? 1 : 0,
      qualityToText(tds.quality), readingAge(tds.sampled_at_ms), ph.value,
      ph.connected ? 1 : 0, qualityToText(ph.quality),
      readingAge(ph.sampled_at_ms), flow.value, flow.connected ? 1 : 0,
      qualityToText(flow.quality), readingAge(flow.sampled_at_ms), lvl1.value,
      lvl1.connected ? 1 : 0, qualityToText(lvl1.quality),
      readingAge(lvl1.sampled_at_ms), lvl2.value, lvl2.connected ? 1 : 0,
      qualityToText(lvl2.quality), readingAge(lvl2.sampled_at_ms), lvl3.value,
      lvl3.connected ? 1 : 0, qualityToText(lvl3.quality),
      readingAge(lvl3.sampled_at_ms), lvl4.value, lvl4.connected ? 1 : 0,
      qualityToText(lvl4.quality), readingAge(lvl4.sampled_at_ms),
      outputs.values[static_cast<size_t>(ActuatorId::CIRCULATION_RELAY)],
      outputs.values[static_cast<size_t>(ActuatorId::AERATOR_RELAY)],
      outputs.values[static_cast<size_t>(ActuatorId::LIGHT_1)],
      outputs.values[static_cast<size_t>(ActuatorId::LIGHT_2)],
      outputs.values[static_cast<size_t>(ActuatorId::FAN_1)],
      outputs.values[static_cast<size_t>(ActuatorId::FAN_2)],
      outputs.values[static_cast<size_t>(ActuatorId::PUMP_A)],
      outputs.values[static_cast<size_t>(ActuatorId::PUMP_B)],
      outputs.values[static_cast<size_t>(ActuatorId::PUMP_PH_DOWN)],
      outputs.values[static_cast<size_t>(ActuatorId::REFILL_PUMP)],
      THRESHOLD_LIGHT_LOW, THRESHOLD_LIGHT_HIGH, THRESHOLD_TEMP_WATER_LOW, THRESHOLD_TEMP_WATER_HIGH,
      THRESHOLD_TEMP_AIR_LOW, THRESHOLD_TEMP_AIR_HIGH, THRESHOLD_HUMIDITY_LOW, THRESHOLD_HUMIDITY_HIGH,
      THRESHOLD_TDS_LOW, THRESHOLD_TDS_HIGH, THRESHOLD_PH_LOW, THRESHOLD_PH_HIGH);

  return String(buffer);
}

void web_api_broadcast_status(WebSocketsServer &web_socket) {
  String status = web_api_build_status_json();
  web_socket.broadcastTXT(status);
}

// ============================================================================
// KHỞI TẠO CÁC ROUTE REST API HTTP & WEBSOCKET EVENT
// ============================================================================
void web_api_init(WebServer &server, WebSocketsServer &web_socket) {
  // Phục vụ Gzip HTML trang chủ
  server.on("/", HTTP_GET, [&server]() {
    server.sendHeader("Content-Encoding", "gzip");
    server.send_P(200, "text/html", reinterpret_cast<const char *>(INDEX_HTML_GZ), INDEX_HTML_GZ_LEN);
  });

  // Phục vụ Gzip CSS
  server.on("/style.css", HTTP_GET, [&server]() {
    server.sendHeader("Content-Encoding", "gzip");
    server.send_P(200, "text/css", reinterpret_cast<const char *>(STYLE_CSS_GZ), STYLE_CSS_GZ_LEN);
  });

  // Phục vụ Gzip JS
  server.on("/script.js", HTTP_GET, [&server]() {
    server.sendHeader("Content-Encoding", "gzip");
    server.send_P(200, "application/javascript", reinterpret_cast<const char *>(SCRIPT_JS_GZ), SCRIPT_JS_GZ_LEN);
  });

  // API lấy JSON trạng thái hệ thống
  server.on("/api/status", HTTP_GET, [&server]() {
    server.send(200, "application/json", web_api_build_status_json());
  });

  // API gửi nhịp tim heartbeat
  server.on("/api/heartbeat", HTTP_POST, [&server]() {
    queueHeartbeat();
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  });

  // API điều khiển thiết bị ngõ ra qua HTTP (POST /api/control)
  server.on("/api/control", HTTP_POST, [&server]() {
    if (!server.hasArg("plain")) {
      server.send(400, "application/json", "{\"error\":\"Missing body\"}");
      return;
    }

    const String body = server.arg("plain");
    ActuatorId id;
    int value = 0;
    bool found = false;

    for (size_t i = 0; i < ACTUATOR_COUNT; ++i) {
      const ActuatorId candidate = static_cast<ActuatorId>(i);
      const String name = actuator_name(candidate);
      if (body.indexOf("\"actuator\":\"" + name + "\"") != -1 ||
          body.indexOf("\"actuator\": \"" + name + "\"") != -1) {
        id = candidate;
        found = true;
        break;
      }
    }

    if (!found) {
      server.send(400, "application/json", "{\"error\":\"Invalid actuator\"}");
      return;
    }

    const int state_idx = body.indexOf("\"state\":");
    if (state_idx != -1) {
      value = body.substring(state_idx + 8).toInt();
    }

    ControlCommand command{};
    command.type = ControlCommandType::SET_OUTPUT;
    command.actuator = id;
    command.value = static_cast<int16_t>(value);
    command.created_at_ms = millis();
    command.ttl_ms = CONTROL_COMMAND_TTL_MS;

    if (app_send_control_command(command, 0)) {
      server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      server.send(503, "application/json", "{\"error\":\"Queue full\"}");
    }
  });

  // API Quét danh sách Wi-Fi lân cận
  server.on("/api/wifi/scan", HTTP_GET, [&server]() {
    const int n = WiFi.scanNetworks();
    String json = "[";
    for (int i = 0; i < n; ++i) {
      if (i > 0) json += ",";
      json += "{\"ssid\":\"" + escapeJson(WiFi.SSID(i)) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
    }
    json += "]";
    server.send(200, "application/json", json);
  });

  // API Lưu SSID/Password Wi-Fi mới vào NVS Flash
  server.on("/api/wifi/save", HTTP_POST, [&server]() {
    if (server.hasArg("ssid")) {
      const String ssid = server.arg("ssid");
      const String pass = server.arg("pass");
      wifi_manager_save_credentials(ssid, pass);
      server.send(200, "application/json", "{\"status\":\"ok\"}");
      delay(100);
      if (!wifi_manager_start_connecting_sta() &&
          !wifi_manager_start_ap()) {
        Serial.println("[WIFI] FATAL: Khong the chuyen STA hoac khoi phuc AP.");
        Serial.flush();
        ESP.restart();
      }
      return;
    }
    server.send(400, "application/json", "{\"error\":\"Missing ssid\"}");
  });

  // API Xóa thông tin Wi-Fi nhà và quay về phát SoftAP
  server.on("/api/wifi/forget", HTTP_POST, [&server]() {
    wifi_manager_clear_credentials();
    server.send(200, "application/json", "{\"status\":\"ok\"}");
    delay(100);
    if (!wifi_manager_start_ap()) {
      Serial.println("[WIFI] FATAL: Khong the khoi phuc AP sau khi xoa Wi-Fi.");
      Serial.flush();
      ESP.restart();
    }
  });

  // Xử lý điều hướng Captive Portal
  server.onNotFound([&server]() {
    if (wifi_manager_get_state() == NetworkState::AP_MODE) {
      server.sendHeader("Location", "http://192.168.4.1/", true);
      server.send(302, "text/plain", "");
      return;
    }
    server.send(404, "text/plain", "Not Found");
  });

  // Lắng nghe sự kiện kết nối & tin nhắn qua WebSocket
  web_socket.onEvent([&web_socket](uint8_t client_num, WStype_t type, uint8_t *payload, size_t length) {
    (void)length;
    switch (type) {
    case WStype_CONNECTED: {
      String status = web_api_build_status_json();
      web_socket.sendTXT(client_num, status);
      queueHeartbeat();
      break;
    }
    case WStype_DISCONNECTED:
      queueAllOff("WS_DISCONNECTED"); // Rớt kết nối WebSocket -> Phát lệnh ngắt tất cả tải
      break;
    case WStype_TEXT: {
      const String message = reinterpret_cast<char *>(payload);
      if (message.indexOf("heartbeat") != -1) {
        queueHeartbeat(); // Nhận nhịp tim từ Web Client -> Gia hạn Web Lease
      } else if (message.indexOf("actuator") != -1) {
        ActuatorId id;
        bool found = false;
        for (size_t i = 0; i < ACTUATOR_COUNT; ++i) {
          const ActuatorId candidate = static_cast<ActuatorId>(i);
          const String name = actuator_name(candidate);
          if (message.indexOf("\"actuator\":\"" + name + "\"") != -1 ||
              message.indexOf("\"actuator\": \"" + name + "\"") != -1) {
            id = candidate;
            found = true;
            break;
          }
        }
        if (found) {
          int value = 0;
          const int state_idx = message.indexOf("\"state\":");
          if (state_idx != -1) {
            value = message.substring(state_idx + 8).toInt();
          }
          ControlCommand command{};
          command.type = ControlCommandType::SET_OUTPUT;
          command.actuator = id;
          command.value = static_cast<int16_t>(value);
          command.created_at_ms = millis();
          command.ttl_ms = CONTROL_COMMAND_TTL_MS;
          app_send_control_command(command, 0);
        }
      }
      break;
    }
    default:
      break;
    }
  });
}
