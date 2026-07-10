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

// --- Theo dõi trạng thái pending để lưu NVS (Settle-time debouncing) ---
struct {
  int pending_value;      // Giá trị chờ lưu
  unsigned long last_change_time; // Thời điểm thay đổi cuối
  bool has_pending;       // Có giá trị đang chờ không?
} static pwm_debounce[8]; // 8 kênh PWM: DEN1, DEN2, QUAT1, QUAT2, BOMLL1-3, BOM12V

// Timeout settle-time: 2 giây không thay đổi → lưu vào NVS
const unsigned long PWM_SETTLE_TIME_MS = 2000;

// --- Hàm ghi/đọc bộ nhớ Flash (NVS Preferences) ---
static void save_actuator_state(int pin, int state) {
  Preferences prefs;
  if (prefs.begin("actuators", false)) {
    char key[8];
    snprintf(key, sizeof(key), "p%d", pin);
    prefs.putInt(key, state);
    prefs.end();
  } else {
    Serial.printf("[ACTUATOR] LOI: Khong the ghi NVS Preferences cho GPIO %d!\n", pin);
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
  } else {
    Serial.printf("[ACTUATOR] LOI: Khong the doc NVS Preferences cho GPIO %d!\n", pin);
  }
  return val;
}

// --- Khởi tạo và tắt toàn bộ các chân ngoại vi (Relay và PWM) ---
void hardware_init() {
  Serial.println("[HARDWARE] Dang khoi tao cac chan ngoai vi...");

  // Bước 1: Khôi phục các biến trạng thái từ bộ nhớ NVS (Flash) TRƯỚC KHI khởi tạo GPIO
  // Điều này giảm thiểu thời gian các thiết bị ở trạng thái sai
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

  // Bước 2: Khởi tạo GPIO pins với kỹ thuật LOW → OUTPUT → Target State
  // Danh sách các chân điều khiển (2 relay + 8 PWM + 3 khác)
  const int output_pins[] = {IN_RL1, IN_RL2, DEN1, DEN2, QUAT1, QUAT2,
                             BOMLL1, BOMLL2, BOMLL3, BOM12V,
                             BUZZER, PIN_EN_TDS, LED_SYS};

  for (int pin : output_pins) {
    digitalWrite(pin, LOW);      // Ghi LOW trước
    pinMode(pin, OUTPUT);        // Chuyển sang OUTPUT
    digitalWrite(pin, LOW);      // Đảm bảo LOW
  }

  // Bước 3: Áp dụng các trạng thái đã khôi phục lên phần cứng vật lý
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

// --- Xử lý lưu giá trị PWM pending sau khi settle (gọi trong loop) ---
void actuator_flush_pending() {
  unsigned long now = millis();
  const int pwm_pins[] = {DEN1, DEN2, QUAT1, QUAT2, BOMLL1, BOMLL2, BOMLL3, BOM12V};

  for (int i = 0; i < 8; i++) {
    if (pwm_debounce[i].has_pending) {
      unsigned long time_since_change = now - pwm_debounce[i].last_change_time;

      // Nếu đã qua 2 giây kể từ lần thay đổi cuối → lưu vào NVS
      if (time_since_change >= PWM_SETTLE_TIME_MS) {
        save_actuator_state(pwm_pins[i], pwm_debounce[i].pending_value);
        pwm_debounce[i].has_pending = false;

        Serial.printf("[ACTUATOR] PWM GPIO %d settled -> Saved to NVS: %d/255\n",
                      pwm_pins[i], pwm_debounce[i].pending_value);
      }
    }
  }
}

// --- Force flush tất cả PWM pending (dùng khi OTA, reset, mất điện...) ---
void actuator_force_flush() {
  const int pwm_pins[] = {DEN1, DEN2, QUAT1, QUAT2, BOMLL1, BOMLL2, BOMLL3, BOM12V};

  for (int i = 0; i < 8; i++) {
    if (pwm_debounce[i].has_pending) {
      save_actuator_state(pwm_pins[i], pwm_debounce[i].pending_value);
      pwm_debounce[i].has_pending = false;
      Serial.printf("[ACTUATOR] Force flush PWM GPIO %d -> NVS: %d/255\n",
                    pwm_pins[i], pwm_debounce[i].pending_value);
    }
  }
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
  bool clamped = false;

  if (is_relay || is_other) {
    val = state ? 1 : 0;
  } else if (is_pwm) {
    if (val < 0) val = 0;
    if (val > 255) val = 255;

    // Nếu giá trị PWM dương nhưng nhỏ hơn mức tối thiểu 10% (khoảng 25/255)
    // và không phải là tắt hẳn (val = 0)
    if (val > 0 && val < 25) {
      clamped = true;
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

  // Áp dụng trạng thái lên phần cứng NGAY LẬP TỨC
  if (is_relay || is_other) {
    digitalWrite(pin, val ? HIGH : LOW);
  } else if (is_pwm) {
    analogWrite(pin, val);
  }

  // Lưu trạng thái vào NVS Flash với Settle-Time Debouncing để giảm mòn flash
  if (is_relay || is_other) {
    // Relay luôn lưu ngay (trạng thái ON/OFF quan trọng)
    save_actuator_state(pin, val);
    Serial.printf("[ACTUATOR] Digital GPIO %d -> %s (Saved NVS)\n",
                  pin, val ? "HIGH" : "LOW");
  } else if (is_pwm) {
    // PWM: Đánh dấu pending, sẽ lưu sau 2s không thay đổi (xử lý trong actuator_flush_pending())
    int pwm_idx = -1;
    if (pin == DEN1) pwm_idx = 0;
    else if (pin == DEN2) pwm_idx = 1;
    else if (pin == QUAT1) pwm_idx = 2;
    else if (pin == QUAT2) pwm_idx = 3;
    else if (pin == BOMLL1) pwm_idx = 4;
    else if (pin == BOMLL2) pwm_idx = 5;
    else if (pin == BOMLL3) pwm_idx = 6;
    else if (pin == BOM12V) pwm_idx = 7;

    if (pwm_idx >= 0) {
      pwm_debounce[pwm_idx].pending_value = val;
      pwm_debounce[pwm_idx].last_change_time = millis();
      pwm_debounce[pwm_idx].has_pending = true;
    }

    Serial.printf("[ACTUATOR] PWM GPIO %d -> %d/255 (%.1f%%)%s (Pending save...)\n",
                  pin, val, (val * 100.0) / 255.0,
                  clamped ? " [Clamped to min 10%]" : "");
  }
}
