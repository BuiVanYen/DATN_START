#include "actuator.h"
#include "config.h"

#include <Arduino.h>

namespace {
// Struct nội bộ định nghĩa thông tin chi tiết của 1 thiết bị ngõ ra
struct ActuatorDefinition {
  ActuatorId id;   // Enum ID định danh
  int pin;         // Số chân GPIO tương ứng
  bool pwm;        // true nếu là kênh PWM, false nếu là Relay ON/OFF
  const char *name;// Tên chuỗi đại diện ("DEN1", "IN_RL1", ...)
};

// Bảng tra cứu toàn cục ánh xạ 10 thiết bị ngõ ra với chân GPIO thực tế
constexpr ActuatorDefinition ACTUATORS[] = {
    {ActuatorId::CIRCULATION_RELAY, IN_RL1, false, "IN_RL1"}, // Relay Bơm tuần hoàn 220V (GPIO7)
    {ActuatorId::AERATOR_RELAY, IN_RL2, false, "IN_RL2"},     // Relay Sục khí 220V (GPIO6)
    {ActuatorId::LIGHT_1, DEN1, true, "DEN1"},                 // PWM Đèn tầng 1 (GPIO17)
    {ActuatorId::LIGHT_2, DEN2, true, "DEN2"},                 // PWM Đèn tầng 2 (GPIO18)
    {ActuatorId::FAN_1, QUAT1, true, "QUAT1"},                 // PWM Quạt tầng 1 (GPIO11)
    {ActuatorId::FAN_2, QUAT2, true, "QUAT2"},                 // PWM Quạt tầng 2 (GPIO10)
    {ActuatorId::PUMP_A, BOMLL1, true, "BOMLL1"},               // PWM Bơm chai A (GPIO13)
    {ActuatorId::PUMP_B, BOMLL2, true, "BOMLL2"},               // PWM Bơm chai B (GPIO12)
    {ActuatorId::PUMP_PH_DOWN, BOMLL3, true, "BOMLL3"},         // PWM Bơm pH Down (GPIO8)
    {ActuatorId::REFILL_PUMP, BOM12V, true, "BOM12V"},          // PWM Bơm cấp nước 12V (GPIO9)
};

// Lấy thông tin định nghĩa thiết bị từ ActuatorId
const ActuatorDefinition *definition(ActuatorId id) {
  const size_t index = static_cast<size_t>(id);
  return index < ACTUATOR_COUNT ? &ACTUATORS[index] : nullptr;
}

// Chuẩn hóa giá trị mong muốn thành giá trị ghi thực tế an toàn
int16_t normalizeValue(const ActuatorDefinition &item, int16_t value) {
  if (!item.pwm) {
    return value == 0 ? 0 : 1; // Nếu là Relay: 0 = OFF, khác 0 = ON (1)
  }

  // Nếu là kênh PWM: Giới hạn trong dải 0 đến 255
  if (value < 0) {
    return 0;
  }
  if (value > 255) {
    return 255;
  }
  // 0 là tắt hoàn toàn; Mức nhỏ hơn PWM_MIN_RUNNING_VALUE (25 ~ 10%) được nâng lên 25
  // để tránh hiện tượng bơm/động cơ chỉ rung rè rè mà không chạy.
  if (value > 0 && value < PWM_MIN_RUNNING_VALUE) {
    return PWM_MIN_RUNNING_VALUE;
  }
  return value;
}
} // namespace

// ============================================================================
// KHỞI TẠO PHẦN CỨNG AN TOÀN KHI KHỞI ĐỘNG (SAFE BOOT)
// ============================================================================
void hardware_init_safe() {
  const int output_pins[] = {IN_RL1, IN_RL2, DEN1, DEN2, QUAT1, QUAT2,
                             BOMLL1, BOMLL2, BOMLL3, BOM12V, BUZZER,
                             LED_SYS};

  // GPIO42 cấp nguồn chung TDS + DS18B20 theo mức active LOW. Khởi tạo
  // riêng ở HIGH để không tạo xung cấp nguồn ngắn trong lúc safe boot.
  digitalWrite(PIN_EN_TDS, HIGH);
  pinMode(PIN_EN_TDS, OUTPUT);
  digitalWrite(PIN_EN_TDS, HIGH);

  // Đặt mức LOW trước khi pinMode OUTPUT để tránh chân ngõ ra bị nháy vô tình khi boot
  for (int pin : output_pins) {
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  // Tắt toàn bộ ngõ ra PWM về 0
  analogWrite(DEN1, 0);
  analogWrite(DEN2, 0);
  analogWrite(QUAT1, 0);
  analogWrite(QUAT2, 0);
  analogWrite(BOMLL1, 0);
  analogWrite(BOMLL2, 0);
  analogWrite(BOMLL3, 0);
  analogWrite(BOM12V, 0);
}

// ============================================================================
// HÀM ÁP DỤNG GIÁ TRỊ GHI NGÕ RA (THỰC THI BỞI TASKSAFETYCONTROL)
// ============================================================================
bool actuator_apply(ActuatorId id, int16_t requested_value,
                    int16_t &applied_value) {
  const ActuatorDefinition *item = definition(id);
  if (item == nullptr) {
    return false;
  }

  applied_value = normalizeValue(*item, requested_value);
  if (item->pwm) {
    analogWrite(item->pin, applied_value); // Ghi xung PWM (0-255)
  } else {
    digitalWrite(item->pin, applied_value == 0 ? LOW : HIGH); // Ghi mức Logic Relay
  }
  return true;
}

// ============================================================================
// NGẮT TẤT CẢ TẢI KHẨN CẤP (ALL OFF)
// ============================================================================
void actuator_all_off() {
  for (const ActuatorDefinition &item : ACTUATORS) {
    if (item.pwm) {
      analogWrite(item.pin, 0);
    } else {
      digitalWrite(item.pin, LOW);
    }
  }
  digitalWrite(BUZZER, LOW);
}

// ============================================================================
// CÁC HÀM HỖ TRỢ TRA CỨU ÁNH XẠ
// ============================================================================
bool actuator_pin_to_id(int pin, ActuatorId &id) {
  for (const ActuatorDefinition &item : ACTUATORS) {
    if (item.pin == pin) {
      id = item.id;
      return true;
    }
  }
  return false;
}

bool actuator_name_to_id(const String &name, ActuatorId &id) {
  for (const ActuatorDefinition &item : ACTUATORS) {
    if (name.equalsIgnoreCase(item.name)) {
      id = item.id;
      return true;
    }
  }
  return false;
}

int actuator_id_to_pin(ActuatorId id) {
  const ActuatorDefinition *item = definition(id);
  return item == nullptr ? -1 : item->pin;
}

bool actuator_is_pwm(ActuatorId id) {
  const ActuatorDefinition *item = definition(id);
  return item != nullptr && item->pwm;
}

const char *actuator_name(ActuatorId id) {
  const ActuatorDefinition *item = definition(id);
  return item == nullptr ? "UNKNOWN" : item->name;
}
