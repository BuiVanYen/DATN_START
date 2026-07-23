#include "mqtt_handler.h"

// ============================================================================
// FUTURE - NOT USED IN MANUAL MODE
// ============================================================================
// MQTT được cố ý để trống trong chế độ MANUAL. Các hàm stub giữ cho
// module có giao diện ổn định nhưng không tạo kết nối, task hoặc log USB.
// Khi phát triển tính năng Cloud IoT (Bản AUTO), logic PubSubClient sẽ được thêm vào đây.
// ============================================================================

void mqtt_init() {}

void mqtt_handle() {}

bool mqtt_is_connected() { return false; }

void mqtt_publish_sensor_data(const char *payload) { (void)payload; }
