#pragma once // Chỉ include file header này 1 lần duy nhất

#include <Arduino.h>

// ============================================================================
// HÀM VÒNG LẶP TASK NETWORK FREERTOS (CHẠY TRÊN CORE 0)
// ============================================================================
// Quản lý kết nối Wi-Fi, WebSocketsServer (port 81), WebServer HTTP (port 80) và Web OTA.
void taskNetwork(void *parameter);

String ota_get_current_ip();
bool ota_is_sta_connected();
