#pragma once
#include <Arduino.h>

// ============================================================
// GPIO Assignment — Đối chiếu từ PCB_CHECKLIST.md (Schematic VER2)
// ============================================================

// --- Relay (kích qua opto-coupler PC817 + transistor 2SC1815) ---

#define IN_RL2 6 // Relay 1: SỤC 220 V (GPIO6, chân 6)
#define IN_RL1 7 // Relay 2: BƠm chìm 220v (GPIO7, chân 7)

// --- PWM MOSFET (kích qua IRLR7843 N-MOSFET, R10kΩ gate) ---
#define DEN1 17   // Relay 1: Đèn LED Tầng 1 (GPIO17, chân 10)
#define DEN2 18   // Relay 2: Đèn LED Tầng 2 (GPIO18, chân 11)
#define QUAT1 11  // PWM1: 2× Quạt Tầng 1 (GPIO11, chân 19)
#define QUAT2 10  // PWM2: 2× Quạt Tầng 2 (GPIO10, chân 18)
#define BOMLL1 13 // PWM3: Bơm DD chai A - pH Up (GPIO13, chân 21)
#define BOMLL2 12 // PWM4: Bơm DD chai B - Nutrient A (GPIO12, chân 20)
#define BOMLL3 8  // pH down
#define BOM12V 9  // PWM6: Bơm nước 12V DC chính (GPIO9, chân 17)

// --- Buzzer & LED hệ thống ---
#define BUZZER 47  // Buzzer qua AO3400A MOSFET (GPIO47, chân 24)
#define LED_SYS 48 // LED hệ thống qua R1kΩ (GPIO48, chân 25)

// --- Cảm biến (khai báo để dùng sau) ---
#define PIN_SDA 5      // I2C SDA (SHT31, BH1750) (GPIO5, chân 5)
#define PIN_SCL 4      // I2C SCL (SHT31, BH1750) (GPIO4, chân 4)
#define PIN_DS18B20 14 // DS18B20 OneWire (GPIO14, chân 22)
#define PIN_ADC_TDS 1  // TDS analog (ADC1_CH1) (GPIO2, chân 38)
#define PIN_EN_TDS 42  // Điều khiển nguồn TDS (GPIO42, chân 35)
#define PIN_ADC_PH 2   // pH analog (ADC1_CH0) (GPIO1, chân 39)
#define PIN_FLOW 41    // Flow sensor (GPIO41, chân 34)
#define PIN_LEVEL1 40  // Mực nước Thùng chính (GPIO40, chân 33)
#define PIN_LEVEL2 39  // Mực nước Chai 1 (GPIO39, chân 32)
#define PIN_LEVEL3 38  // Mực nước Chai 2 (GPIO38, chân 31)
#define PIN_LEVEL4 21  // Mực nước Chai 3 (GPIO21, chân 23)

// Khởi tạo trạng thái tắt cho tất cả ngoại vi
// (xuất mức LOW để tắt MOSFET/Relay vì mạch không có trở kéo xuống GND)
void hardware_init();
