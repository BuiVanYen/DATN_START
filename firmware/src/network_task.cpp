#include "network_task.h"

#include "config.h"
#include "ota_update.h"
#include "rtos_app.h"
#include "web_api.h"
#include "wifi_manager.h"

#include <WebServer.h>
#include <WebSocketsServer.h>

namespace {
WebServer server(OTA_PORT);       // Khởi tạo máy chủ HTTP cổng 80
WebSocketsServer web_socket(WS_PORT); // Khởi tạo máy chủ WebSocket cổng 81

uint32_t state_timer_ms = 0;      // Timer đo thời gian trạng thái kết nối
uint32_t led_timer_ms = 0;        // Timer đo chu kỳ nhấp nháy LED chỉ thị
bool led_state = false;           // Trạng thái bật/tắt của LED
bool wifi_lost_flag = false;      // Cờ ghi nhận mất kết nối Wi-Fi
uint32_t wifi_lost_at_ms = 0;     // Thời điểm mất kết nối Wi-Fi
uint32_t last_broadcast_ms = 0;   // Timer broadcast WebSocket

NetworkState previous_state = NetworkState::AP_MODE; // Lưu trạng thái mạng trước đó để phát hiện chuyển đổi

[[noreturn]] void restartAfterNetworkFailure(const char *reason) {
  Serial.print("[NETWORK] FATAL: ");
  Serial.println(reason);
  Serial.flush();
  digitalWrite(LED_SYS, LOW);
  vTaskDelay(pdMS_TO_TICKS(1000));
  ESP.restart();
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// Điều khiển nhấp nháy LED trạng thái hệ thống (LED_SYS - GPIO48)
void updateStatusLed(NetworkState state, uint32_t now) {
  if (state == NetworkState::STA_CONNECTED) {
    led_state = true;
    digitalWrite(LED_SYS, HIGH); // Kết nối thành công: LED sáng liên tục
    return;
  }

  const uint32_t interval_ms =
      state == NetworkState::STA_CONNECTING ? 100 : 500;
  if (now - led_timer_ms >= interval_ms) {
    led_timer_ms = now;
    led_state = !led_state;
    digitalWrite(LED_SYS, led_state ? HIGH : LOW);
  }
}
} // namespace

String ota_get_current_ip() {
  return wifi_manager_get_current_ip();
}

bool ota_is_sta_connected() {
  return wifi_manager_is_sta_connected();
}

// ============================================================================
// CHÍNH: VÒNG LẶP TASK NETWORK FREERTOS (CHẠY ĐỘC LẬP TRÊN CORE 0)
// ============================================================================
void taskNetwork(void *parameter) {
  (void)parameter;

  Serial.println("[NETWORK] TaskNetwork started.");

  // 1. Khởi tạo Wi-Fi Manager, HTTP REST API và Web OTA Update
  wifi_manager_init();
  web_api_init(server, web_socket);
  ota_update_init(server);

  // 2. Bật Wi-Fi trước để khởi tạo esp_netif/LwIP. Chỉ sau đó mới được
  // mở socket HTTP/WebSocket.
  bool wifi_ready = false;
  if (!wifi_manager_get_ssid().isEmpty()) {
    wifi_ready = wifi_manager_start_connecting_sta();
    if (wifi_ready) {
      state_timer_ms = millis();
      previous_state = NetworkState::STA_CONNECTING;
    } else {
      Serial.println("[NETWORK] STA init failed, fallback to AP.");
      wifi_ready = wifi_manager_start_ap();
      previous_state = NetworkState::AP_MODE;
      wifi_lost_flag = false;
    }
  } else {
    wifi_ready = wifi_manager_start_ap();
    previous_state = NetworkState::AP_MODE;
    wifi_lost_flag = false;
  }

  if (!wifi_ready) {
    restartAfterNetworkFailure("Wi-Fi init failed");
  }

  server.begin();
  web_socket.begin();
  Serial.println("[NETWORK] HTTP :80 and WebSocket :81 ready.");

  uint32_t last_health_ms = 0;

  for (;;) {
    const uint32_t now = millis();
    wifi_manager_process_dns(); // Xử lý bẫy Captive Portal

    NetworkState state = wifi_manager_get_state();

    // Reset timer khi vừa chuyển từ trạng thái này sang trạng thái khác
    if (state != previous_state) {
      if (state == NetworkState::STA_CONNECTING) {
        state_timer_ms = now; // Reset timer khi vừa rớt mạng sang STA_CONNECTING (tránh timeout tức thì)
      }
      previous_state = state;
    }

    // Logic chuyển đổi trạng thái kết nối Wi-Fi
    if (state == NetworkState::STA_CONNECTING) {
      if (now - state_timer_ms >= WIFI_RECONNECT_TIMEOUT_MS) {
        if (wifi_manager_start_ap()) { // Quá 15 giây không nối được -> Chuyển sang AP Mode
          previous_state = NetworkState::AP_MODE;
          wifi_lost_flag = false;
        } else {
          restartAfterNetworkFailure("STA timeout and AP fallback failed");
        }
      }
    } else if (state == NetworkState::STA_CONNECTED) {
      wifi_lost_flag = false;
    } else if (state == NetworkState::AP_MODE) {
      if (!wifi_manager_get_ssid().isEmpty()) {
        if (!wifi_lost_flag) {
          wifi_lost_flag = true;
          wifi_lost_at_ms = now;
        } else if (now - wifi_lost_at_ms >= STA_RETRY_INTERVAL_MS) {
          if (wifi_manager_start_connecting_sta()) { // Mỗi 30 giây thử nối lại Wi-Fi nhà 1 lần
            state_timer_ms = now;
            previous_state = NetworkState::STA_CONNECTING;
          } else {
            Serial.println("[NETWORK] STA retry init failed, staying in AP.");
            if (wifi_manager_start_ap()) {
              previous_state = NetworkState::AP_MODE;
              wifi_lost_flag = false;
            } else {
              restartAfterNetworkFailure("STA retry and AP restore failed");
            }
          }
        }
      }
    }

    // 3. Xử lý yêu cầu từ Client Web và WebSocket
    server.handleClient();
    web_socket.loop();

    // 4. Phát Broadcast dữ liệu cảm biến & ngõ ra JSON qua WebSocket mỗi 1 giây
    if (now - last_broadcast_ms >= WS_BROADCAST_MS) {
      last_broadcast_ms = now;
      web_api_broadcast_status(web_socket);
    }

    // 5. Cập nhật nhấp nháy LED trạng thái
    updateStatusLed(state, now);

    // 6. Gửi báo cáo nhịp tim sức khỏe Task (Heartbeat) mỗi 1000ms
    if (now - last_health_ms >= 1000) {
      last_health_ms = now;
      app_update_task_health(TaskRole::NETWORK, now, xPortGetCoreID(),
                             uxTaskGetStackHighWaterMark(nullptr));
    }

    vTaskDelay(pdMS_TO_TICKS(10)); // Tạm dừng 10ms nhường CPU cho các tiến trình hệ thống
  }
}
