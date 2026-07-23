#include "failsafe.h"

#include "actuator.h"
#include "config.h"
#include "rtos_app.h"

namespace {
// Cập nhật trạng thái ngõ ra hiện tại vào cấu trúc ActuatorSnapshot trong RAM
void publishOutputs(const int16_t values[ACTUATOR_COUNT], bool lease_active,
                    const char *reason) {
  static uint32_t sequence = 0;
  ActuatorSnapshot snapshot{};
  for (size_t i = 0; i < ACTUATOR_COUNT; ++i) {
    snapshot.values[i] = values[i];
  }
  snapshot.mode = SystemMode::MANUAL;
  snapshot.all_off = true;
  for (int16_t value : snapshot.values) {
    if (value != 0) {
      snapshot.all_off = false;
      break;
    }
  }
  snapshot.web_lease_active = lease_active;
  snapshot.sequence = ++sequence;
  strlcpy(snapshot.last_reason, reason, sizeof(snapshot.last_reason));
  app_set_actuator_snapshot(snapshot);
}

// Ngắt tất cả 10 thiết bị phần cứng thực tế và vô hiệu hóa cờ Web Lease
void switchAllOff(int16_t values[ACTUATOR_COUNT], bool &lease_active,
                  const char *reason) {
  actuator_all_off();
  for (size_t i = 0; i < ACTUATOR_COUNT; ++i) {
    values[i] = 0;
  }
  lease_active = false;
  publishOutputs(values, lease_active, reason);
}
} // namespace

// ============================================================================
// CHÍNH: VÒNG LẶP TASK SAFETY CONTROL FREERTOS (CORE 1, PRIORITY 5)
// ============================================================================
void taskSafetyControl(void *parameter) {
  (void)parameter;

  int16_t output_values[ACTUATOR_COUNT] = {}; // Mảng lưu giá trị thực tế của 10 ngõ ra
  bool web_lease_active = false;             // Cờ hiệu lực của Web Lease (Thời gian sống kết nối)
  uint32_t last_web_activity_ms = 0;         // Thời điểm nhận tương tác Web gần nhất
  uint32_t last_health_ms = 0;               // Timer gửi nhịp tim sức khỏe Task
  char reason[64] = "MANUAL_READY";          // Chuỗi lý do điều khiển

  // Khởi động an toàn: Tắt tất cả ngõ ra ban đầu
  actuator_all_off();
  publishOutputs(output_values, web_lease_active, reason);

  for (;;) {
    ControlCommand command{};

    // Lắng nghe lệnh từ CommandQueue (chờ tối đa 100ms để kiểm tra lease định kỳ)
    if (app_receive_control_command(command, pdMS_TO_TICKS(100))) {
      // Phải lấy thời gian SAU khi nhận lệnh. Nếu lấy trước lúc chờ Queue, lệnh
      // vừa được tạo có created_at_ms lớn hơn mốc cũ; phép trừ uint32_t sẽ
      // underflow và biến lệnh mới thành lệnh "quá hạn".
      const uint32_t received_at_ms = millis();

      // Kiểm tra lệnh có bị quá thời hạn sống TTL (5 giây) hay không
      const bool command_expired =
          static_cast<uint32_t>(received_at_ms - command.created_at_ms) >
          command.ttl_ms;

      if (!command_expired) {
        switch (command.type) {
        case ControlCommandType::SET_OUTPUT: { // Lệnh bật/tắt Relay hoặc chỉnh PWM
          const size_t index = static_cast<size_t>(command.actuator);
          int16_t applied_value = 0;
          if (index < ACTUATOR_COUNT &&
              actuator_apply(command.actuator, command.value, applied_value)) {
            output_values[index] = applied_value;
            web_lease_active = true; // Kích hoạt / Gia hạn Web Lease
            last_web_activity_ms = received_at_ms;
            strlcpy(reason, "MANUAL_COMMAND", sizeof(reason));
          }
          break;
        }
        case ControlCommandType::HEARTBEAT: // Lệnh nhịp tim gia hạn Lease từ Web
          web_lease_active = true;
          last_web_activity_ms = received_at_ms;
          strlcpy(reason, "MANUAL_HEARTBEAT", sizeof(reason));
          break;
        case ControlCommandType::ALL_OFF: // Lệnh ngắt khẩn cấp khi mất Wi-Fi/WebSocket
          strlcpy(reason, "NETWORK_DISCONNECTED", sizeof(reason));
          switchAllOff(output_values, web_lease_active, reason);
          break;
        case ControlCommandType::OTA_START: // Lệnh ngắt khẩn cấp chuẩn bị nạp OTA
          strlcpy(reason, "OTA_SAFE_OFF", sizeof(reason));
          switchAllOff(output_values, web_lease_active, reason);
          break;
        }
      }
    }

    const uint32_t now = millis();

    // Kiểm tra Web Lease Timeout: Nếu quá 10 giây (`MANUAL_WEB_LEASE_MS`) không có heartbeat, TỰ TẮT TOÀN BỘ TẢI
    if (web_lease_active && now - last_web_activity_ms > MANUAL_WEB_LEASE_MS) {
      strlcpy(reason, "WEB_LEASE_EXPIRED", sizeof(reason));
      switchAllOff(output_values, web_lease_active, reason);
    } else {
      publishOutputs(output_values, web_lease_active, reason);
    }

    // ========================================================================
    // FUTURE - NOT USED IN MANUAL MODE:
    // SensorQueue được tiêu thụ ở đây để giải phóng dung lượng Queue (tránh tràn Queue),
    // đồng thời kiểm tra tín hiệu sống từ TaskSensors. Trong chế độ MANUAL hiện tại,
    // TaskSafetyControl KHÔNG tự động bật/tắt thiết bị theo dữ liệu cảm biến này.
    // ========================================================================
    SensorUpdate sensor_update{};
    while (app_receive_sensor_update(sensor_update, 0)) {
      // Đã giải phóng 1 mẩu tin sensor update từ Queue.
    }

    // Gửi báo cáo sức khỏe Task (Heartbeat) mỗi 1000ms
    if (now - last_health_ms >= 1000) {
      last_health_ms = now;
      app_update_task_health(TaskRole::CONTROL, now, xPortGetCoreID(),
                             uxTaskGetStackHighWaterMark(nullptr));
    }
  }
}
