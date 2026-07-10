#pragma once
#include <Arduino.h>

// --- Khởi tạo hệ thống WiFi và OTA Web Server ---
void ota_init();

// --- Xử lý liên tục các sự kiện WiFi, DNS, WebServer, LED trong loop() ---
void ota_handle();

// --- Trả về IP hiện tại (dạng chuỗi) ---
String ota_get_current_ip();

// --- Kiểm tra xem có đang kết nối WiFi thành công không ---
bool ota_is_sta_connected();
