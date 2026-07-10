#include "config.h"
#include <Preferences.h>

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

// --- Hàm ghi/đọc bộ nhớ Flash (NVS Preferences) ---
static void save_actuator_state(int pin, int state) {
  Preferences prefs;
  if (prefs.begin("actuators", false)) {
    char key[8];
    snprintf(key, sizeof(key), "p%d", pin);
    prefs.putInt(key, state);
    prefs.end();
  }
}

static int load_actuator_state(int pin, int default_val) {
  Preferences prefs;
  int val = default_val;
  if (prefs.begin("actuators", true)) {
    char key[8];
    snprintf(key, sizeof(key), "p%d", pin);
    val = prefs.getInt(key, default_val);
    prefs.end();
  }
  return val;
}

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

  // Khôi phục các biến trạng thái từ bộ nhớ NVS (Flash)
  state_IN_RL1 = load_actuator_state(IN_RL1, 0);
  state_IN_RL2 = load_actuator_state(IN_RL2, 0);
  state_DEN1 = load_actuator_state(DEN1, 0);
  state_DEN2 = load_actuator_state(DEN2, 0);
  state_QUAT1 = load_actuator_state(QUAT1, 0);
  state_QUAT2 = load_actuator_state(QUAT2, 0);
  state_BOMLL1 = load_actuator_state(BOMLL1, 0);
  state_BOMLL2 = load_actuator_state(BOMLL2, 0);
  state_BOMLL3 = load_actuator_state(BOMLL3, 0);
  state_BOM12V = load_actuator_state(BOM12V, 0);
  state_BUZZER = load_actuator_state(BUZZER, 0);
  state_PIN_EN_TDS = load_actuator_state(PIN_EN_TDS, 0);
  state_LED_SYS = load_actuator_state(LED_SYS, 0);

  // Áp dụng các trạng thái đã khôi phục lên phần cứng vật lý
  digitalWrite(IN_RL1, state_IN_RL1 ? HIGH : LOW);
  digitalWrite(IN_RL2, state_IN_RL2 ? HIGH : LOW);
  digitalWrite(BUZZER, state_BUZZER ? HIGH : LOW);
  digitalWrite(PIN_EN_TDS, state_PIN_EN_TDS ? HIGH : LOW);
  digitalWrite(LED_SYS, state_LED_SYS ? HIGH : LOW);

  auto apply_pwm = [](int pin, int val) {
    if (val > 0 && val < 25) val = 25; // Áp dụng giới hạn tối thiểu 10%
    analogWrite(pin, val);
  };
  apply_pwm(DEN1, state_DEN1);
  apply_pwm(DEN2, state_DEN2);
  apply_pwm(QUAT1, state_QUAT1);
  apply_pwm(QUAT2, state_QUAT2);
  apply_pwm(BOMLL1, state_BOMLL1);
  apply_pwm(BOMLL2, state_BOMLL2);
  apply_pwm(BOMLL3, state_BOMLL3);
  apply_pwm(BOM12V, state_BOM12V);

  Serial.println("[HARDWARE] Da phuc hoi trang thai cu tu NVS:");
  Serial.printf("  RL1: %d, RL2: %d\n", state_IN_RL1, state_IN_RL2);
  Serial.printf("  DEN1: %d, DEN2: %d\n", state_DEN1, state_DEN2);
  Serial.printf("  QUAT1: %d, QUAT2: %d\n", state_QUAT1, state_QUAT2);
  Serial.printf("  BOMLL1: %d, BOMLL2: %d, BOMLL3: %d, BOM12V: %d\n", state_BOMLL1, state_BOMLL2, state_BOMLL3, state_BOM12V);
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

  // Tiền xử lý và chuẩn hóa giá trị trạng thái
  int val = state;
  if (is_relay || is_other) {
    val = state ? 1 : 0;
  } else if (is_pwm) {
    if (val < 0) val = 0;
    if (val > 255) val = 255;
    
    // Nếu giá trị PWM dương nhưng nhỏ hơn mức tối thiểu 10% (khoảng 25/255)
    // và không phải là tắt hẳn (val = 0)
    if (val > 0 && val < 25) {
      val = 25; // Giới hạn tối thiểu 10%
    }
  }

  // Cập nhật biến lưu trữ trạng thái cục bộ
  switch (pin) {
    case IN_RL1: state_IN_RL1 = val; break;
    case IN_RL2: state_IN_RL2 = val; break;
    case DEN1: state_DEN1 = val; break;
    case DEN2: state_DEN2 = val; break;
    case QUAT1: state_QUAT1 = val; break;
    case QUAT2: state_QUAT2 = val; break;
    case BOMLL1: state_BOMLL1 = val; break;
    case BOMLL2: state_BOMLL2 = val; break;
    case BOMLL3: state_BOMLL3 = val; break;
    case BOM12V: state_BOM12V = val; break;
    case BUZZER: state_BUZZER = val; break;
    case PIN_EN_TDS: state_PIN_EN_TDS = val; break;
    case LED_SYS: state_LED_SYS = val; break;
  }

  // Lưu trạng thái mới vào bộ nhớ Flash (NVS) để duy trì khi mất nguồn/khởi động lại
  save_actuator_state(pin, val);

  // Áp dụng trạng thái lên phần cứng
  if (is_relay || is_other) {
    digitalWrite(pin, val ? HIGH : LOW);
    Serial.printf("[ACTUATOR] Digital GPIO %d -> %s (Saved NVS)\n", pin, val ? "HIGH" : "LOW");
  } else if (is_pwm) {
    analogWrite(pin, val);
    Serial.printf("[ACTUATOR] PWM GPIO %d -> %d/255 (%.1f%%) (Saved NVS)\n", pin, val, (val * 100.0) / 255.0);
  }
}
