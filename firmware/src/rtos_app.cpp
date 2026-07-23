#include "rtos_app.h"

#include "config.h"
#include <cstring>

namespace {
// Biến con trỏ toàn cục quản lý Hàng chờ (Queue) của FreeRTOS
QueueHandle_t command_queue = nullptr; // Hàng chờ truyền lệnh từ Task Network sang Task Control
QueueHandle_t sensor_queue = nullptr;  // Hàng chờ truyền thông báo lấy mẫu từ Task Sensors

// Biến con trỏ toàn cục quản lý Khóa bộ nhớ (Mutex) chống tranh chấp dữ liệu
SemaphoreHandle_t sensor_mutex = nullptr;   // Bảo vệ cấu trúc SensorSnapshot
SemaphoreHandle_t actuator_mutex = nullptr; // Bảo vệ cấu trúc ActuatorSnapshot
SemaphoreHandle_t health_mutex = nullptr;   // Bảo vệ cấu trúc RuntimeHealth

// Cấu trúc bộ nhớ Snapshots lưu trong RAM
SensorSnapshot sensor_snapshot{};     // Bản chụp 11 cảm biến
ActuatorSnapshot actuator_snapshot{}; // Bản chụp 10 thiết bị ngoại vi
RuntimeHealth runtime_health{};       // Bản chụp sức khỏe 3 Task

// Hàm Helper nội bộ: Xin cấp quyền xin khóa Mutex trong tối đa 20ms
bool takeMutex(SemaphoreHandle_t mutex) {
  return mutex != nullptr && xSemaphoreTake(mutex, pdMS_TO_TICKS(20)) == pdTRUE;
}
} // namespace

// ============================================================================
// KHỞI TẠO TÀI NGUYÊN FREERTOS (QUEUE & MUTEX)
// ============================================================================
bool app_rtos_init() {
  // Tạo Queue chứa tối đa COMMAND_QUEUE_LENGTH mẩu tin kiểu ControlCommand
  command_queue = xQueueCreate(COMMAND_QUEUE_LENGTH, sizeof(ControlCommand));
  // Tạo Queue chứa tối đa SENSOR_QUEUE_LENGTH mẩu tin kiểu SensorUpdate
  sensor_queue = xQueueCreate(SENSOR_QUEUE_LENGTH, sizeof(SensorUpdate));

  // Tạo 3 Mutex bảo vệ các vùng nhớ dùng chung
  sensor_mutex = xSemaphoreCreateMutex();
  actuator_mutex = xSemaphoreCreateMutex();
  health_mutex = xSemaphoreCreateMutex();

  // Kiểm tra nếu tạo thất bại (thiếu RAM) thì trả về false
  if (command_queue == nullptr || sensor_queue == nullptr ||
      sensor_mutex == nullptr || actuator_mutex == nullptr ||
      health_mutex == nullptr) {
    return false;
  }

  // Khởi tạo trạng thái mặc định cho 11 cảm biến (Chất lượng STALE)
  for (size_t i = 0; i < SENSOR_COUNT; ++i) {
    sensor_snapshot.readings[i] = {
        .value = 0.0F,
        .quality = SensorQuality::STALE,
        .sampled_at_ms = 0,
        .raw_value = 0,
        .connected = false,
    };
  }

  // Khởi tạo trạng thái mặc định cho 10 thiết bị ngoại vi (Tất cả OFF)
  actuator_snapshot.mode = SystemMode::MANUAL;
  actuator_snapshot.all_off = true;
  actuator_snapshot.web_lease_active = false;
  strlcpy(actuator_snapshot.last_reason, "SAFE_BOOT", sizeof(actuator_snapshot.last_reason));
  return true;
}

// ============================================================================
// HÀM TRUYỀN / NHẬN LỆNH ĐIỀU KHIỂN QUA COMMAND QUEUE
// ============================================================================
bool app_send_control_command(const ControlCommand &command, TickType_t wait_ticks) {
  return command_queue != nullptr &&
         xQueueSend(command_queue, &command, wait_ticks) == pdTRUE;
}

bool app_receive_control_command(ControlCommand &command, TickType_t wait_ticks) {
  return command_queue != nullptr &&
         xQueueReceive(command_queue, &command, wait_ticks) == pdTRUE;
}

// ============================================================================
// HÀM CẬP NHẬT CẢM BIẾN VÀ ĐẨY VÀO SENSOR QUEUE
// ============================================================================
void app_publish_sensor(const SensorUpdate &update) {
  // 1. Cập nhật dữ liệu vào bản chụp SensorSnapshot trong RAM (Có khóa Mutex)
  if (takeMutex(sensor_mutex)) {
    const size_t index = static_cast<size_t>(update.id);
    if (index < SENSOR_COUNT) {
      sensor_snapshot.readings[index] = update.reading;
      sensor_snapshot.sequence++;
    }
    xSemaphoreGive(sensor_mutex); // Trả khóa Mutex ngay sau khi ghi xong
  }

  // 2. Đẩy thông báo vào SensorQueue để thông báo cho TaskControl tiêu thụ.
  // Nếu Queue bị đầy, loại bỏ tin nhắn cũ nhất và chèn tin nhắn mới vào để tránh làm nghẽn TaskSensors.
  if (sensor_queue != nullptr && xQueueSend(sensor_queue, &update, 0) != pdTRUE) {
    SensorUpdate discarded{};
    xQueueReceive(sensor_queue, &discarded, 0); // Bỏ mẩu tin cũ nhất
    xQueueSend(sensor_queue, &update, 0);       // Đẩy mẩu tin mới vào
  }
}

bool app_receive_sensor_update(SensorUpdate &update, TickType_t wait_ticks) {
  return sensor_queue != nullptr &&
         xQueueReceive(sensor_queue, &update, wait_ticks) == pdTRUE;
}

// ============================================================================
// HÀM ĐỌC / GHI AN TOÀN BẢN CHỤP SNAPSHOTS
// ============================================================================
void app_get_sensor_snapshot(SensorSnapshot &snapshot) {
  if (takeMutex(sensor_mutex)) {
    snapshot = sensor_snapshot;
    xSemaphoreGive(sensor_mutex);
  }
}

void app_set_actuator_snapshot(const ActuatorSnapshot &snapshot) {
  if (takeMutex(actuator_mutex)) {
    actuator_snapshot = snapshot;
    xSemaphoreGive(actuator_mutex);
  }
}

void app_get_actuator_snapshot(ActuatorSnapshot &snapshot) {
  if (takeMutex(actuator_mutex)) {
    snapshot = actuator_snapshot;
    xSemaphoreGive(actuator_mutex);
  }
}

// ============================================================================
// HÀM THEO DÕI SỨC KHỎE THI HÀNH TASK (HEALTH MONITOR)
// ============================================================================
void app_update_task_health(TaskRole role, uint32_t heartbeat_ms,
                             int8_t core, uint32_t stack_words) {
  if (!takeMutex(health_mutex)) {
    return;
  }

  switch (role) {
  case TaskRole::NETWORK:
    runtime_health.network_heartbeat_ms = heartbeat_ms;
    runtime_health.network_core = core;
    runtime_health.network_stack_words = stack_words;
    break;
  case TaskRole::SENSORS:
    runtime_health.sensors_heartbeat_ms = heartbeat_ms;
    runtime_health.sensors_core = core;
    runtime_health.sensors_stack_words = stack_words;
    break;
  case TaskRole::CONTROL:
    runtime_health.control_heartbeat_ms = heartbeat_ms;
    runtime_health.control_core = core;
    runtime_health.control_stack_words = stack_words;
    break;
  }

  xSemaphoreGive(health_mutex);
}

void app_get_runtime_health(RuntimeHealth &health) {
  if (takeMutex(health_mutex)) {
    health = runtime_health;
    xSemaphoreGive(health_mutex);
  }
}

UBaseType_t app_command_queue_depth() {
  return command_queue == nullptr ? 0 : uxQueueMessagesWaiting(command_queue);
}

UBaseType_t app_sensor_queue_depth() {
  return sensor_queue == nullptr ? 0 : uxQueueMessagesWaiting(sensor_queue);
}
