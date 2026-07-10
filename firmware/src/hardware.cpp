#include "hardware.h"

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
