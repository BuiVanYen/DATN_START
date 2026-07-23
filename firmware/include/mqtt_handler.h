#pragma once // Chỉ include file header này 1 lần duy nhất

// ============================================================================
// FUTURE - NOT USED IN MANUAL MODE
// ============================================================================
// Các hàm stub MQTT bên dưới được giữ lại để chuẩn bị cho giao diện kết nối
// Cloud MQTT trong phiên bản AUTO nâng cao sau này. Trong phiên bản MANUAL hiện
// tại, các hàm này là rỗng và không tạo bất kỳ kết nối mạng nào.
// ============================================================================

void mqtt_init();
void mqtt_handle();
bool mqtt_is_connected();
void mqtt_publish_sensor_data(const char *payload);
