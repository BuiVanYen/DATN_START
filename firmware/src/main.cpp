#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "thresholds.h"
#include "ota_handler.h"

void setup() {
  // 1. Khởi động giao tiếp Serial debug
  Serial.begin(115200);
  delay(500);

  // Khởi tạo và tắt toàn bộ các chân ngoại vi (Relay và PWM) ngay lập tức
  hardware_init();

  Serial.println("\n=============================================");
  Serial.println("         ECOFARM - ESP32 CONTROL CENTER      ");
  Serial.println("=============================================");

  // 2. Khởi tạo các cảm biến trước khi chạy hệ thống WiFi/OTA
  sensors_init();

  // 3. Khởi tạo hệ thống WiFi tự động + OTA Web Server
  ota_init();

  Serial.println("[SYSTEM] He thong da khoi dong xong!");
}

void loop() {
  // 1. Chạy ngầm các tác vụ kiểm soát WiFi, Captive Portal DNS, và WebServer OTA
  ota_handle();

  // 2. Xử lý flush các giá trị PWM pending sau khi settle (2s không thay đổi)
  actuator_flush_pending();
}
