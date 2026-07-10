#include "config.h"

// --- Biến lưu trữ trạng thái hiện tại của từng chân ---
static int state_IN_RL1 = 0;
static int state_IN_RL2 = 0;
static int state_DEN1 = 0;
static int state_DEN2 = 0;
static int state_QUAT1 = 0;
static int state_QUAT2 = 0;
static int state_BOMLL1 = 0;
static int state_BOMLL2 = 0;
static int state_BOMLL3 = 0;
static int state_BOM12V = 0;
static int state_BUZZER = 0;
static int state_PIN_EN_TDS = 0;
static int state_LED_SYS = 0;

// --- Khởi tạo và tắt toàn bộ các chân ngoại vi (Relay và PWM) ---
void hardware_init() {
  Serial.println("[HARDWARE] Dang khoi tao cac chan ngoai vi...");

  // Danh sách các chân điều khiển Relay (2 kênh) và PWM MOSFET (8 kênh)
  // Tất cả cần được kéo xuống LOW ngay lập tức để tránh tải ngoại vi tự bật
  // do mạch PCB không có trở pull-down trên gate MOSFET / base transistor
  const int output_pins[] = {// 2 kênh Relay
                             IN_RL1, IN_RL2,
                             // 8 kênh PWM MOSFET
                             DEN1, DEN2, QUAT1, QUAT2, BOMLL1, BOMLL2, BOMLL3, BOM12V,
                             // Ngoại vi khác
                             BUZZER, PIN_EN_TDS, LED_SYS};

  // Kỹ thuật: Ghi LOW trước → sau đó mới chuyển sang OUTPUT
  // Tránh chân bị nháy glitch HIGH trong quá trình chuyển mode
  for (int pin : output_pins) {
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW); // Đảm bảo chắc chắn LOW sau khi set OUTPUT
  }

  // Khởi tạo các biến trạng thái
  state_IN_RL1 = 0; state_IN_RL2 = 0;
  state_DEN1 = 0; state_DEN2 = 0;
  state_QUAT1 = 0; state_QUAT2 = 0;
  state_BOMLL1 = 0; state_BOMLL2 = 0; state_BOMLL3 = 0;
  state_BOM12V = 0;
  state_BUZZER = 0; state_PIN_EN_TDS = 0; state_LED_SYS = 0;

  Serial.println("[HARDWARE] Da tat toan bo 2 Relay + 8 kenh PWM MOSFET + Ngoai vi.");
}

// --- Lấy trạng thái hiện tại của ngoại vi ---
int actuator_get_state(int pin) {
  switch (pin) {
    case IN_RL1: return state_IN_RL1;
    case IN_RL2: return state_IN_RL2;
    case DEN1: return state_DEN1;
    case DEN2: return state_DEN2;
    case QUAT1: return state_QUAT1;
    case QUAT2: return state_QUAT2;
    case BOMLL1: return state_BOMLL1;
    case BOMLL2: return state_BOMLL2;
    case BOMLL3: return state_BOMLL3;
    case BOM12V: return state_BOM12V;
    case BUZZER: return state_BUZZER;
    case PIN_EN_TDS: return state_PIN_EN_TDS;
    case LED_SYS: return state_LED_SYS;
    default: return 0;
  }
}

// --- Điều khiển trạng thái ngoại vi (Relay và PWM) ---
void actuator_set_state(int pin, int state) {
  bool is_relay = (pin == IN_RL1 || pin == IN_RL2);
  bool is_pwm = (pin == DEN1 || pin == DEN2 || pin == QUAT1 || pin == QUAT2 || 
                 pin == BOMLL1 || pin == BOMLL2 || pin == BOMLL3 || pin == BOM12V);
  bool is_other = (pin == BUZZER || pin == PIN_EN_TDS || pin == LED_SYS);

  if (!is_relay && !is_pwm && !is_other) {
    Serial.printf("[ACTUATOR] CANH BAO: Ghi sai chan ngoai vi GPIO %d\n", pin);
    return;
  }

  // Cập nhật biến lưu trữ trạng thái cục bộ
  switch (pin) {
    case IN_RL1: state_IN_RL1 = state ? 1 : 0; break;
    case IN_RL2: state_IN_RL2 = state ? 1 : 0; break;
    case DEN1: state_DEN1 = state; break;
    case DEN2: state_DEN2 = state; break;
    case QUAT1: state_QUAT1 = state; break;
    case QUAT2: state_QUAT2 = state; break;
    case BOMLL1: state_BOMLL1 = state; break;
    case BOMLL2: state_BOMLL2 = state; break;
    case BOMLL3: state_BOMLL3 = state; break;
    case BOM12V: state_BOM12V = state; break;
    case BUZZER: state_BUZZER = state ? 1 : 0; break;
    case PIN_EN_TDS: state_PIN_EN_TDS = state ? 1 : 0; break;
    case LED_SYS: state_LED_SYS = state ? 1 : 0; break;
  }

  if (is_relay || is_other) {
    // Thiết bị On/Off (Mức HIGH/LOW)
    digitalWrite(pin, state ? HIGH : LOW);
    Serial.printf("[ACTUATOR] Digital GPIO %d -> %s\n", pin, state ? "HIGH" : "LOW");
  } else if (is_pwm) {
    // Thiết bị điều chế xung PWM (0 - 255)
    int val = state;
    if (val < 0) val = 0;
    if (val > 255) val = 255;
    
    // Nếu giá trị PWM dương nhưng nhỏ hơn mức tối thiểu 10% (khoảng 25/255)
    // và không phải là tắt hẳn (val = 0)
    if (val > 0 && val < 25) {
      val = 25; // Giới hạn tối thiểu 10%
    }

    analogWrite(pin, val);
    Serial.printf("[ACTUATOR] PWM GPIO %d -> %d/255 (%.1f%%)\n", pin, val, (val * 100.0) / 255.0);
  }
}
