#include <Arduino.h>

#include "actuator.h"
#include "config.h"
#include "failsafe.h"
#include "ota_handler.h"
#include "rtos_app.h"
#include "sensors.h"

#include <esp_system.h>

namespace {
// Thẻ quản lý (Task Handle) của 3 Task FreeRTOS
TaskHandle_t network_task_handle = nullptr;
TaskHandle_t sensors_task_handle = nullptr;
TaskHandle_t control_task_handle = nullptr;

[[noreturn]] void restartAfterBootFailure(const char *reason) {
  actuator_all_off();
  digitalWrite(PIN_EN_TDS, HIGH);
  digitalWrite(LED_SYS, LOW);
  Serial.printf("[BOOT] FATAL: %s; free_heap=%u\n", reason,
                static_cast<unsigned>(ESP.getFreeHeap()));
  Serial.flush();
  delay(1000);
  ESP.restart();
  for (;;) {
    delay(1000);
  }
}
} // namespace

// ============================================================================
// HÀM KHỞI ĐỘNG HỆ THỐNG (CHẠY 1 LẦN KHI CẤP NGUỒN HOẶC RESET)
// ============================================================================
void setup() {
  // 1. Khởi tạo ngõ ra an toàn: Đưa toàn bộ 10 thiết bị về mức TẮT (OFF) trước khi làm bất kỳ việc gì khác
  hardware_init_safe();

  // Mở kênh chẩn đoán sau khi các ngõ ra đã được đưa về trạng thái an toàn.
  Serial.begin(115200);
  delay(250);
  Serial.printf("\n[BOOT] firmware=%s reset_reason=%d free_heap=%u\n",
                FW_VERSION, static_cast<int>(esp_reset_reason()),
                static_cast<unsigned>(ESP.getFreeHeap()));

  // 2. Khởi tạo tài nguyên FreeRTOS (Tạo 2 Queue và 3 Mutex)
  if (!app_rtos_init()) {
    restartAfterBootFailure("RTOS queue/mutex init failed");
  }

  // 3. Tạo TaskSafetyControl trước để ngõ ra luôn có chủ sở hữu an toàn
  // trước khi Web bắt đầu nhận lệnh.
  if (xTaskCreatePinnedToCore(taskSafetyControl, "TaskSafetyControl",
                              STACK_CONTROL, nullptr, PRIORITY_CONTROL,
                              &control_task_handle, CORE_CONTROL) != pdPASS) {
    restartAfterBootFailure("TaskSafetyControl create failed");
  }
  Serial.println("[BOOT] TaskSafetyControl: OK");

  // 4. Tạo TaskSensors đọc định kỳ 11 cảm biến trên Core 1.
  // - Gán chạy trên Core 1 (CORE_SENSORS)
  // - Stack: 6144 Bytes (STACK_SENSORS)
  // - Độ ưu tiên: 3 (PRIORITY_SENSORS)
  if (xTaskCreatePinnedToCore(taskSensors, "TaskSensors", STACK_SENSORS,
                              nullptr, PRIORITY_SENSORS, &sensors_task_handle,
                              CORE_SENSORS) != pdPASS) {
    restartAfterBootFailure("TaskSensors create failed");
  }
  Serial.println("[BOOT] TaskSensors: OK");

  // 5. Chỉ mở TaskNetwork sau khi TaskSafetyControl và TaskSensors đã tồn tại.
  if (xTaskCreatePinnedToCore(taskNetwork, "TaskNetwork", STACK_NETWORK,
                              nullptr, PRIORITY_NETWORK, &network_task_handle,
                              CORE_NETWORK) != pdPASS) {
    restartAfterBootFailure("TaskNetwork create failed");
  }
  Serial.println("[BOOT] TaskNetwork: OK");

  Serial.println("[BOOT] All tasks created.");
}

// ============================================================================
// VÒNG LẶP ARDUINO MẶC ĐỊNH (MAIN LOOP)
// ============================================================================
// Do toàn bộ công việc đã được 3 Task FreeRTOS đảm nhận độc lập,
// vòng loop() chỉ cần ngủ vTaskDelay 1000ms để không tốn CPU.
void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
