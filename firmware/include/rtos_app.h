#pragma once // Chỉ include file header này 1 lần duy nhất

#include "system_types.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// ============================================================================
// KHAI BÁO CÁC HÀM GIAO TIẾP ĐA NHÂN VÀ QUẢN LÝ BỘ NHỚ FREERTOS
// ============================================================================

// Khởi tạo tất cả tài nguyên FreeRTOS (Queue và Mutex). Trả về true nếu thành công.
bool app_rtos_init();

// Đẩy một lệnh điều khiển vào CommandQueue (TaskNetwork gọi để gửi sang TaskControl)
bool app_send_control_command(const ControlCommand &command, TickType_t wait_ticks = 0);

// Lấy một lệnh điều khiển từ CommandQueue (TaskControl gọi để lấy lệnh ra xử lý)
bool app_receive_control_command(ControlCommand &command, TickType_t wait_ticks);

// Đẩy dữ liệu đọc 1 cảm biến vào SensorQueue và cập nhật bản chụp SensorSnapshot trong RAM
void app_publish_sensor(const SensorUpdate &update);

// Lấy mẩu tin cập nhật cảm biến từ SensorQueue
bool app_receive_sensor_update(SensorUpdate &update, TickType_t wait_ticks);

// Đọc an toàn bản chụp toàn bộ 11 cảm biến từ RAM (Có dùng khóa Mutex bảo vệ)
void app_get_sensor_snapshot(SensorSnapshot &snapshot);

// Ghi an toàn bản chụp trạng thái 10 thiết bị ngoại vi vào RAM (Có dùng khóa Mutex bảo vệ)
void app_set_actuator_snapshot(const ActuatorSnapshot &snapshot);

// Đọc an toàn bản chụp trạng thái 10 thiết bị ngoại vi từ RAM
void app_get_actuator_snapshot(ActuatorSnapshot &snapshot);

// Cập nhật tín hiệu sức khỏe (Heartbeat, Core, Dung lượng Stack còn dư) của từng Task
void app_update_task_health(TaskRole role, uint32_t heartbeat_ms, int8_t core, uint32_t stack_words);

// Lấy thông tin tổng hợp sức khỏe hệ thống của cả 3 Task FreeRTOS
void app_get_runtime_health(RuntimeHealth &health);

// Kiểm tra số lượng tin nhắn đang chờ trong CommandQueue
UBaseType_t app_command_queue_depth();

// Kiểm tra số lượng tin nhắn đang chờ trong SensorQueue
UBaseType_t app_sensor_queue_depth();
