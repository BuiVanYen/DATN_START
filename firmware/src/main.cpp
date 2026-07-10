#include "hardware.h"
#include "ota.h"
// #include "sensors.h"
#include <Arduino.h>
#include <WiFi.h>

void setup() {
  // 1. Khởi động giao tiếp Serial debug
  Serial.begin(115200);
  delay(500);

  // Khởi tạo và tắt toàn bộ các chân ngoại vi (Relay và PWM) ngay lập tức
  hardware_init();

  Serial.println("\n=============================================");
  Serial.println("         ECOFARM - ESP32 CONTROL CENTER      ");
  Serial.println("=============================================");

  // 2. Khởi tạo hệ thống WiFi tự động + OTA Web Server
  ota_init();

  // 3. Khởi tạo các cảm biến (Tạm thời tắt để mai code lại)
  // sensors_init();

  Serial.println("[SYSTEM] He thong da khoi dong xong!");
}

void loop() {
  // 1. Chạy ngầm các tác vụ kiểm soát WiFi, Captive Portal DNS, và WebServer
  // OTA
  ota_handle();

  // 2. Đọc và hiển thị cường độ ánh sáng mỗi 5 giây (Tạm thời tắt để mai code
  // lại)
  /*
  static unsigned long last_sensor_read = 0;
  if (millis() - last_sensor_read >= 5000) {
    last_sensor_read = millis();
    float lux = sensors_read_light();
    Serial.printf("[LUX] Cường độ ánh sáng: %.2f Lux\n", lux);
  }
  */
}
