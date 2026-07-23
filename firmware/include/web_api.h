#pragma once // Chỉ include file header này 1 lần duy nhất

#include <Arduino.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

// Khởi tạo các Route HTTP REST API và Đăng ký sự kiện WebSocket
void web_api_init(WebServer &server, WebSocketsServer &web_socket);

// Hàm dựng chuỗi JSON tổng hợp toàn bộ trạng thái hệ thống (Cảm biến, Ngõ ra, Mạng)
String web_api_build_status_json();

// Phát broadcast dữ liệu trạng thái JSON tới tất cả client đang kết nối WebSocket
void web_api_broadcast_status(WebSocketsServer &web_socket);
