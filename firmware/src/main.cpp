#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "thresholds.h"
#include "ota_handler.h"

// --- Khởi tạo và tắt toàn bộ các chân ngoại vi (Relay và PWM) ---
void hardware_init() {
  Serial.println("[HARDWARE] Dang khoi tao cac chan ngoai vi...");

  // Danh sách các chân điều khiển Relay (2 kênh) và PWM MOSFET (8 kênh)
  // Tất cả cần được kéo xuống LOW ngay lập tức để tránh tải ngoại vi tự bật
  // do mạch PCB không có trở pull-down trên gate MOSFET / base transistor
  const int output_pins[] = {// 2 kênh Relay
                             IN_RL1, IN_RL2,
                             // 8 kênh PWM MOSFET
                             DEN1, DEN2, QUAT1, QUAT2, BOMLL1, BOMLL2, BOMLL3, BOM12V,
                             // Ngoại vi khác
                             BUZZER, PIN_EN_TDS};

  // Kỹ thuật: Ghi LOW trước → sau đó mới chuyển sang OUTPUT
  // Tránh chân bị nháy glitch HIGH trong quá trình chuyển mode
  for (int pin : output_pins) {
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW); // Đảm bảo chắc chắn LOW sau khi set OUTPUT
  }

  Serial.println("[HARDWARE] Da tat toan bo 2 Relay + 8 kenh PWM MOSFET.");
  Serial.printf("[HARDWARE] Relay (2CH): GPIO %d, %d\n", IN_RL1, IN_RL2);
  Serial.printf("[HARDWARE] PWM (8CH):   GPIO %d, %d, %d, %d, %d, %d, %d, %d\n",
                DEN1, DEN2, QUAT1, QUAT2, BOMLL1, BOMLL2, BOMLL3, BOM12V);
}

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

  // 3. Khởi tạo các cảm biến
  sensors_init();

  Serial.println("[SYSTEM] He thong da khoi dong xong!");
}

void loop() {
  // 1. Chạy ngầm các tác vụ kiểm soát WiFi, Captive Portal DNS, và WebServer OTA
  ota_handle();

  // 2. Đọc và hiển thị cường độ ánh sáng mỗi 5 giây
  static unsigned long last_sensor_read = 0;
  if (millis() - last_sensor_read >= 5000) {
    last_sensor_read = millis();
    float lux = sensors_read_light();
    Serial.printf("[LUX] Cuong do anh sang: %.2f Lux\n", lux);
  }
}
