#pragma once // Định hướng trình biên dịch chỉ include header này 1 lần duy nhất

#include "system_types.h"

// ============================================================================
// HÀM KHỞI TẠO PHẦN CỨNG AN TOÀN KHI VỪA CẤP NGUỒN (SAFE BOOT)
// ============================================================================
// Đặt tất cả 10 ngõ ra (2 Relay + 8 PWM) về mức TẮT (0).
// Hàm này bắt buộc phải gọi đầu tiên trong main.cpp trước khi bật Wi-Fi hay FreeRTOS Tasks.
void hardware_init_safe();

// ============================================================================
// HÀM GHI AN TOÀN NGÕ RA PHẦN CỨNG (CHỈ CÓ TASKSAFETYCONTROL ĐƯỢC GỌI)
// ============================================================================
// Áp dụng giá trị mong muốn (requested_value) lên thiết bị id.
// Trả về applied_value thực tế sau khi đã ép dải an toàn (0 đến 255) hoặc ngưỡng tối thiểu.
bool actuator_apply(ActuatorId id, int16_t requested_value, int16_t &applied_value);

// Tắt ngay lập tức toàn bộ 10 thiết bị ngõ ra
void actuator_all_off();

// ============================================================================
// CÁC HÀM TRA CỨU ÁNH XẠ THIẾT BỊ NGOẠI VI
// ============================================================================
// Ánh xạ từ số chân GPIO vật lý sang enum ActuatorId
bool actuator_pin_to_id(int pin, ActuatorId &id);

// Ánh xạ từ tên mã chuỗi (Ví dụ "DEN1", "BOMLL1", "IN_RL1") sang enum ActuatorId
bool actuator_name_to_id(const String &name, ActuatorId &id);

// Tra cứu chân GPIO tương ứng của 1 ActuatorId
int actuator_id_to_pin(ActuatorId id);

// Kiểm tra xem thiết bị có phải là kênh băm xung PWM hay không (True=PWM, False=Relay)
bool actuator_is_pwm(ActuatorId id);

// Lấy tên chuỗi hiển thị của 1 ActuatorId (dùng cho debug/JSON)
const char *actuator_name(ActuatorId id);
