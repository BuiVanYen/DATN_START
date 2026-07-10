#pragma once
#include <Arduino.h>

// ============================================================
// THÔNG TIN PHIÊN BẢN FIRMWARE & HỆ THỐNG
// ============================================================
#define FW_VERSION "1.0.4"
#define AP_SSID "DATN_AIOT_LETTUCE"
#define OTA_PORT 80
#define WS_PORT 81

// ============================================================
// GPIO Pin Assignment (Đối chiếu từ PCB_CHECKLIST.md)
// ============================================================

// --- Relay (kích qua opto-coupler PC817 + transistor 2SC1815) ---
#define IN_RL2 6 // Relay 1: SỤC 220 V (GPIO6)
#define IN_RL1 7 // Relay 2: Bơm chìm 220v (GPIO7)

// --- PWM MOSFET ---
#define DEN1 17   // Đèn LED Tầng 1 (GPIO17)
#define DEN2 18   // Đèn LED Tầng 2 (GPIO18)
#define QUAT1 11  // PWM1: Quạt Tầng 1 (GPIO11)
#define QUAT2 10  // PWM2: Quạt Tầng 2 (GPIO10)
#define BOMLL1 13 // PWM3: Bơm DD chai A - pH Up (GPIO13)
#define BOMLL2 12 // PWM4: Bơm DD chai B - Nutrient A (GPIO12)
#define BOMLL3 8  // pH down (GPIO8)
#define BOM12V 9  // PWM6: Bơm nước 12V DC chính (GPIO9)

// --- Buzzer & LED hệ thống ---
#define BUZZER 47  // Buzzer (GPIO47)
#define LED_SYS 48 // LED hệ thống (GPIO48)

// --- Cảm biến ---
#define PIN_SDA 5      // I2C SDA (GPIO5)
#define PIN_SCL 4      // I2C SCL (GPIO4)
#define PIN_DS18B20 14 // DS18B20 OneWire (GPIO14)
#define PIN_ADC_TDS 1  // TDS analog
#define PIN_EN_TDS 42  // Nguồn TDS (GPIO42)
#define PIN_ADC_PH 2   // pH analog
#define PIN_FLOW 41    // Flow sensor (GPIO41)
#define PIN_LEVEL1 40  // Mực nước Thùng chính (GPIO40)
#define PIN_LEVEL2 39  // Mực nước Chai 1 (GPIO39)
#define PIN_LEVEL3 38  // Mực nước Chai 2 (GPIO38)
#define PIN_LEVEL4 21  // Mực nước Chai 3 (GPIO21)

// ============================================================
// KHAI BÁO CÁC HÀM HỆ THỐNG
// ============================================================

// Khởi tạo phần cứng (Relay, MOSFET...)
void hardware_init();

// Điều khiển trạng thái ngoại vi
void actuator_set_state(int pin, int state);
int actuator_get_state(int pin);

// Khởi tạo cảm biến
void sensors_init();

// Đọc giá trị cường độ ánh sáng (Lux)
float sensors_read_light();

// Kiểm tra kết nối BH1750
bool sensors_is_bh1750_connected();
