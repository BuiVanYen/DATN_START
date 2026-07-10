#ifndef OTA_H
#define OTA_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "hardware.h"

// --- Cấu hình chân LED hệ thống ---
#define SYS_LED_PIN LED_SYS

// --- Cấu hình cổng Web Server ---
#define OTA_PORT 80

// --- Khởi tạo hệ thống WiFi và OTA Web Server ---
void ota_init();

// --- Xử lý liên tục các sự kiện WiFi, DNS, WebServer, LED trong loop() ---
void ota_handle();

// --- Trả về IP hiện tại (dạng chuỗi) ---
String ota_get_current_ip();

// --- Kiểm tra xem có đang kết nối WiFi thành công không ---
bool ota_is_sta_connected();

#endif // OTA_H
