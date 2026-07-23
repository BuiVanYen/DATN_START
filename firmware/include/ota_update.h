#pragma once // Chỉ include file header này 1 lần duy nhất

#include <Arduino.h>
#include <WebServer.h>

// Khởi tạo các đường dẫn HTTP cho phép nạp code từ xa qua Web OTA (/update)
void ota_update_init(WebServer &server);
void ota_update_handle_upload(WebServer &server);
void ota_update_handle_end(WebServer &server);

// Lấy thông báo lỗi nếu quá trình nạp OTA thất bại
String ota_update_get_error_text();
