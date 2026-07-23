#include "sensors.h"

#include "config.h"
#include "rtos_app.h"

#include <Adafruit_SHT31.h>
#include <BH1750.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wire.h>
#include <cmath>

namespace {
// ============================================================================
// HẰNG SỐ CẢM BIẾN VÀ CÔNG THỨC VẬT LÝ (SENSOR CONSTANTS)
// ============================================================================
namespace SensorConstants {
// Cấu hình ADC ESP32-S3 (12-bit = 0..4095, VREF = 3.3V)
constexpr float ADC_RESOLUTION_MAX = 4095.0F;   // Độ phân giải cực đại ADC 12-bit
constexpr float ADC_VREF_VOLTS = 3.3F;          // [V] Điện áp tham chiếu ADC
constexpr uint16_t ADC_RAW_VALID_MIN = 50;      // Ngưỡng ADC tối thiểu dùng riêng cho phép kiểm tra pH
constexpr uint16_t ADC_RAW_VALID_MAX = 4050;    // Ngưỡng bão hòa cao của ADC

// Cảm biến lưu lượng (Flow Sensor YF-S201: 1-30L/min, F = 7.5 * Q)
constexpr float FLOW_PULSE_FACTOR = 7.5F;        // [Hz / (L/min)] Tỉ lệ đếm xung ngắt sang lưu lượng lít/phút (YF-S201 spec)
constexpr float FLOW_MIN_LPM = 1.0F;             // [L/min] Ngưỡng đo tối thiểu theo datasheet YF-S201 (1 L/phút)
constexpr float FLOW_MAX_LPM = 30.0F;            // [L/min] Ngưỡng đo tối đa theo datasheet YF-S201 (30 L/phút)

// Cảm biến pH (Module PH0-14 + Cầu phân áp PCB)
constexpr float PH_PCB_DIVIDER_RATIO = 1.6667F;  // Tỉ lệ cầu phân áp trên PCB
constexpr float PH_NEUTRAL_VOLTAGE = 2.5F;        // [V] Điện áp chuẩn tại pH 7.0
constexpr float PH_NEUTRAL_VALUE = 7.0F;          // Giá trị pH trung tính
constexpr float PH_VOLTAGE_SLOPE = 0.18F;         // [V/pH] Độ dốc điện áp theo độ pH
constexpr float PH_MAX_SAMPLE_DELTA = 0.15F;       // [pH] Ngưỡng chênh lệch pH tối đa cho phép giữa 2 lần lấy mẫu

// Cảm biến TDS (Gravity TDS Meter Module E3601344: 0-1000 PPM, 0-2.3V)
constexpr float TDS_TEMP_COEFF = 0.02F;          // [1/°C] Hệ số bù nhiệt độ (2%/°C)
constexpr float TDS_NEUTRAL_TEMP_C = 25.0F;      // [°C] Nhiệt độ chuẩn tham chiếu 25°C
constexpr float TDS_POLY_COEFF_A = 133.42F;      // Hệ số đa thức A bậc 3
constexpr float TDS_POLY_COEFF_B = 255.86F;      // Hệ số đa thức B bậc 2
constexpr float TDS_POLY_COEFF_C = 857.39F;      // Hệ số đa thức C bậc 1
constexpr float TDS_PPM_SCALE_FACTOR = 0.5F;     // Tỉ lệ chuyển đổi độ dẫn uS/cm sang nồng độ PPM
constexpr float TDS_MAX_PPM = 1000.0F;           // [PPM] Dải đo chuẩn theo datasheet E3601344 (0-1000 PPM)
constexpr float TDS_STABILITY_MAX_DIFF_RATIO = 0.05F; // Ngưỡng chênh lệch nhiễu tối đa 5%
} // namespace SensorConstants

// Đối tượng phần cứng thư viện
BH1750 light_meter;                  // Cảm biến ánh sáng I2C BH1750
Adafruit_SHT31 sht31;                // Cảm biến nhiệt độ/độ ẩm I2C SHT31
OneWire one_wire(PIN_DS18B20);       // Giao thức OneWire cho DS18B20
DallasTemperature ds18b20(&one_wire);// Thư viện đọc DS18B20

// Biến trạng thái nội bộ
uint8_t bh1750_addr = 0x23;          // Địa chỉ I2C mặc định của BH1750 (0x23 hoặc 0x5C)
bool bh1750_connected = false;       // Cờ trạng thái kết nối BH1750
bool sht31_connected = false;        // Cờ trạng thái kết nối SHT31
bool ds18b20_connected = false;      // Cờ trạng thái kết nối DS18B20
float last_good_lux = 0.0F;          // Lưu giá trị Lux tốt nhất gần nhất
uint8_t bh1750_fail_count = 0;       // Đếm số lần đọc lỗi liên tiếp BH1750

volatile uint32_t flow_pulses = 0;  // Biến đếm xung ngắt cảm biến Flow (volatile vì dùng trong ISR)
portMUX_TYPE flow_mux = portMUX_INITIALIZER_UNLOCKED; // Mutex ngắt cờ an toàn cho FreeRTOS

// Biến chống nảy (Debounce) cho 4 phao nước
float debounced_levels[4] = {0.0F, 0.0F, 0.0F, 0.0F};
float pending_levels[4] = {0.0F, 0.0F, 0.0F, 0.0F};
uint32_t level_changed_at[4] = {0, 0, 0, 0};

// Ngắt Interrupt khi có xung từ cảm biến lưu lượng nước YF-S201
void IRAM_ATTR flowPulseCounter() {
  portENTER_CRITICAL_ISR(&flow_mux);
  flow_pulses++;
  portEXIT_CRITICAL_ISR(&flow_mux);
}

// Đọc trạng thái Logic phao nước (Active LOW: LOW = 1.0/Còn nước, HIGH = 0.0/Cạn nước)
float readLevelPin(int pin) {
  return digitalRead(pin) == LOW ? 1.0F : 0.0F;
}

// Đóng gói mẩu tin SensorReading
SensorReading makeReading(float value, SensorQuality quality, bool connected, uint16_t raw = 0) {
  return {
      .value = value,
      .quality = quality,
      .sampled_at_ms = millis(),
      .raw_value = raw,
      .connected = connected,
  };
}

// Đẩy dữ liệu cảm biến vào RAM và Queue
void publish(SensorId id, const SensorReading &reading) {
  SensorUpdate update{};
  update.id = id;
  update.reading = reading;
  app_publish_sensor(update);
}

// Hàm sắp xếp mảng nhỏ (Bubble Sort) dùng cho thuật toán lọc trung vị Trimmed Mean
void sortSmallArray(uint16_t *items, size_t count) {
  for (size_t i = 1; i < count; ++i) {
    const uint16_t key = items[i];
    int j = static_cast<int>(i) - 1;
    while (j >= 0 && items[j] > key) {
      items[j + 1] = items[j];
      --j;
    }
    items[j + 1] = key;
  }
}

// Thuật toán đọc ADC lọc nhiễu nâng cao (Trimmed Mean Filter):
// Lấy 30 mẫu ADC, sắp xếp tăng dần, loại bỏ 20% mẫu cực đại/cực tiểu và tính trung bình 60% mẫu còn lại.
float readTrimmedAdc(int pin, uint16_t &raw_average) {
  constexpr size_t SAMPLE_COUNT = 30;
  constexpr size_t DROP_COUNT = 6; // Bỏ 6 mẫu thấp nhất và 6 mẫu cao nhất
  uint16_t samples[SAMPLE_COUNT];

  for (size_t i = 0; i < SAMPLE_COUNT; ++i) {
    samples[i] = static_cast<uint16_t>(analogRead(pin));
    vTaskDelay(pdMS_TO_TICKS(2));
  }

  sortSmallArray(samples, SAMPLE_COUNT);

  uint32_t sum = 0;
  for (size_t i = DROP_COUNT; i < SAMPLE_COUNT - DROP_COUNT; ++i) {
    sum += samples[i];
  }

  raw_average = static_cast<uint16_t>(sum / (SAMPLE_COUNT - DROP_COUNT * 2));
  return static_cast<float>(raw_average);
}

// Tự động giải phóng và khôi phục Bus I2C khi bị treo thiết bị (Bus Recovery)
void recoverI2cBus() {
  Wire.end();
  pinMode(PIN_SDA, INPUT_PULLUP);
  pinMode(PIN_SCL, OUTPUT_OPEN_DRAIN);

  for (uint8_t i = 0; i < 9; ++i) {
    if (digitalRead(PIN_SDA) == HIGH) {
      break;
    }
    digitalWrite(PIN_SCL, LOW);
    delayMicroseconds(5);
    digitalWrite(PIN_SCL, HIGH);
    delayMicroseconds(5);
  }

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(400000);
  Wire.setTimeOut(10);
  light_meter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, bh1750_addr);
}

// Khởi tạo phần cứng tất cả các cảm biến
void initSensorsHardware() {
  pinMode(PIN_LEVEL1, INPUT);
  pinMode(PIN_LEVEL2, INPUT);
  pinMode(PIN_LEVEL3, INPUT);
  pinMode(PIN_LEVEL4, INPUT);

  // Cảm biến lưu lượng YF-S201 dùng Cổng NPN Open-Collector (kéo về GND khi có xung).
  // Cần bật INPUT_PULLUP để kéo mức HIGH khi transistor ngắt và bắt cạnh xuống FALLING.
  pinMode(PIN_FLOW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_FLOW), flowPulseCounter, FALLING);

  pinMode(PIN_EN_TDS, OUTPUT);
  digitalWrite(PIN_EN_TDS, HIGH); // Tắt nguồn chung TDS + DS18B20 ban đầu

  analogReadResolution(12); // Độ phân giải ADC 12-bit (0-4095)
  analogSetPinAttenuation(PIN_ADC_TDS, ADC_11db); // Dải đo ADC 0 - 3.3V
  analogSetPinAttenuation(PIN_ADC_PH, ADC_11db);

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(400000); // Tốc độ I2C Fast-Mode 400kHz
  Wire.setTimeOut(10);

  // Quét nhanh địa chỉ I2C của BH1750
  for (uint8_t address : {static_cast<uint8_t>(0x23), static_cast<uint8_t>(0x5C)}) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      bh1750_addr = address;
      break;
    }
  }

  bh1750_connected = light_meter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, bh1750_addr);
  sht31_connected = sht31.begin(0x44);

  // DS18B20 dùng chung rail 5V_TDS đang tắt tại đây. Chỉ quét OneWire sau
  // khi bật rail trong sampleWaterGroup().
}

// Đọc 4 phao mực nước kèm thuật toán chống nảy 300ms
void sampleLevels(uint32_t now) {
  const int pins[4] = {PIN_LEVEL1, PIN_LEVEL2, PIN_LEVEL3, PIN_LEVEL4};
  const SensorId ids[4] = {SensorId::LEVEL_MAIN, SensorId::LEVEL_A,
                           SensorId::LEVEL_B, SensorId::LEVEL_PH};

  for (size_t i = 0; i < 4; ++i) {
    const float raw = readLevelPin(pins[i]);
    if (raw != pending_levels[i]) {
      pending_levels[i] = raw;
      level_changed_at[i] = now;
    }

    if (debounced_levels[i] != pending_levels[i] &&
        now - level_changed_at[i] >= LEVEL_DEBOUNCE_MS) {
      debounced_levels[i] = pending_levels[i];
    }

    publish(ids[i], makeReading(debounced_levels[i], SensorQuality::GOOD, true,
                                static_cast<uint16_t>(raw)));
  }
}

// Tính toán lưu lượng nước từ số xung ngắt Interrupt (YF-S201: 1 - 30 L/phút, F = 7.5 * Q)
void sampleFlow(uint32_t now, uint32_t &last_flow_ms) {
  const uint32_t duration_ms = now - last_flow_ms;
  if (duration_ms < FLOW_SAMPLE_MS) {
    return;
  }

  portENTER_CRITICAL(&flow_mux);
  const uint32_t pulses = flow_pulses;
  flow_pulses = 0;
  portEXIT_CRITICAL(&flow_mux);

  last_flow_ms = now;

  float liters_per_minute = 0.0F;
  SensorQuality quality = SensorQuality::GOOD;

  if (pulses > 0) {
    const float hz = static_cast<float>(pulses) / (duration_ms / 1000.0F);
    liters_per_minute = hz / SensorConstants::FLOW_PULSE_FACTOR; // Công thức YF-S201: L/min = Hz / 7.5

    // Không xóa xung thật. Ngoài dải 1-30 L/phút vẫn giữ giá trị để chẩn đoán,
    // nhưng đánh dấu UNSTABLE vì nằm ngoài dải đo được nhà sản xuất bảo đảm.
    if (liters_per_minute < SensorConstants::FLOW_MIN_LPM ||
        liters_per_minute > SensorConstants::FLOW_MAX_LPM) {
      quality = SensorQuality::UNSTABLE;
    }
  }

  publish(SensorId::FLOW, makeReading(liters_per_minute, quality, true,
                                      pulses > 65535 ? 65535 : static_cast<uint16_t>(pulses)));
}

// Đọc cảm biến ánh sáng BH1750
void sampleBh1750() {
  Wire.beginTransmission(bh1750_addr);
  byte err = Wire.endTransmission();

  if (err != 0) {
    const uint8_t alt_addr = bh1750_addr == 0x23 ? 0x5C : 0x23;
    Wire.beginTransmission(alt_addr);
    if (Wire.endTransmission() == 0) {
      bh1750_addr = alt_addr;
      bh1750_connected = light_meter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, bh1750_addr);
      err = bh1750_connected ? 0 : 1;
    }
  }

  if (err != 0) {
    bh1750_fail_count++;
    if (bh1750_fail_count >= 3) {
      recoverI2cBus(); // Khôi phục bus nếu lỗi 3 lần liên tiếp
      bh1750_fail_count = 0;
      bh1750_connected = false;
    }
    publish(SensorId::LIGHT, makeReading(last_good_lux, SensorQuality::STALE, bh1750_connected));
    return;
  }

  if (!bh1750_connected) {
    bh1750_connected = light_meter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, bh1750_addr);
  }

  const float lux = light_meter.readLightLevel();
  if (lux < 0.0F) {
    bh1750_fail_count++;
    bh1750_connected = bh1750_fail_count < 3;
    publish(SensorId::LIGHT, makeReading(last_good_lux, SensorQuality::STALE, bh1750_connected));
    return;
  }

  bh1750_fail_count = 0;
  bh1750_connected = true;
  last_good_lux = lux;
  publish(SensorId::LIGHT, makeReading(lux, SensorQuality::GOOD, true));
}

// Đọc cảm biến nhiệt độ & độ ẩm không khí SHT31
void sampleSht31() {
  if (!sht31_connected) {
    sht31_connected = sht31.begin(0x44);
  }

  const float temp = sht31.readTemperature();
  const float humidity = sht31.readHumidity();
  const bool temp_ok = !isnan(temp);
  const bool humidity_ok = !isnan(humidity);
  sht31_connected = temp_ok && humidity_ok;

  publish(SensorId::TEMP_AIR, makeReading(temp_ok ? temp : -127.0F, temp_ok ? SensorQuality::GOOD : SensorQuality::ERROR, temp_ok));
  publish(SensorId::HUMIDITY, makeReading(humidity_ok ? humidity : -1.0F, humidity_ok ? SensorQuality::GOOD : SensorQuality::ERROR, humidity_ok));
}

// Đọc cảm biến pH dung dịch
void samplePh() {
  uint16_t raw_1 = 0;
  uint16_t raw_2 = 0;
  const float adc_1 = readTrimmedAdc(PIN_ADC_PH, raw_1);
  const float adc_2 = readTrimmedAdc(PIN_ADC_PH, raw_2);
  const float adc_avg = (adc_1 + adc_2) / 2.0F;

  // Tính điện áp v_adc -> v_sensor (qua cầu phân áp 1.6667) -> tính giá trị pH
  const float v_adc = (adc_avg / SensorConstants::ADC_RESOLUTION_MAX) * SensorConstants::ADC_VREF_VOLTS;
  const float v_sensor = v_adc * SensorConstants::PH_PCB_DIVIDER_RATIO;
  float ph = SensorConstants::PH_NEUTRAL_VALUE + ((SensorConstants::PH_NEUTRAL_VOLTAGE - v_sensor) / SensorConstants::PH_VOLTAGE_SLOPE);

  if (ph < 0.0F) {
    ph = 0.0F;
  }
  if (ph > 14.0F) {
    ph = 14.0F;
  }

  const float diff_ph = std::fabs(adc_1 - adc_2) * SensorConstants::ADC_VREF_VOLTS / SensorConstants::ADC_RESOLUTION_MAX * SensorConstants::PH_PCB_DIVIDER_RATIO / SensorConstants::PH_VOLTAGE_SLOPE;
  const bool raw_ok = raw_1 > SensorConstants::ADC_RAW_VALID_MIN &&
                      raw_1 < SensorConstants::ADC_RAW_VALID_MAX &&
                      raw_2 > SensorConstants::ADC_RAW_VALID_MIN &&
                      raw_2 < SensorConstants::ADC_RAW_VALID_MAX;
  const bool stable = diff_ph <= SensorConstants::PH_MAX_SAMPLE_DELTA;
  SensorQuality quality = SensorQuality::GOOD;
  if (!raw_ok) {
    quality = SensorQuality::ERROR;
  } else if (!stable) {
    quality = SensorQuality::UNSTABLE;
  }

  publish(SensorId::PH, makeReading(ph, quality, raw_ok, static_cast<uint16_t>(adc_avg)));
}

// Đọc nhóm cảm biến nước (DS18B20 + TDS)
void sampleWaterGroup() {
  // 1. Cấp nguồn cho mạch đo TDS/Nhiệt độ (GPIO42 active LOW)
  digitalWrite(PIN_EN_TDS, LOW);
  vTaskDelay(pdMS_TO_TICKS(150)); // Chờ 150ms cho điện áp mạch TDS ổn định

  // 2. Đọc nhiệt độ nước từ DS18B20
  ds18b20.begin();
  ds18b20.requestTemperatures();
  float water_temp = ds18b20.getTempCByIndex(0);
  ds18b20_connected = !(water_temp == DEVICE_DISCONNECTED_C || water_temp == -127.0F ||
                        water_temp < -20.0F || water_temp > 80.0F);

  if (!ds18b20_connected) {
    water_temp = SensorConstants::TDS_NEUTRAL_TEMP_C; // Nếu DS18B20 lỗi, tạm dùng nhiệt độ chuẩn 25°C để bù TDS
  }

  publish(SensorId::TEMP_WATER, makeReading(ds18b20_connected ? water_temp : -127.0F,
                                             ds18b20_connected ? SensorQuality::GOOD : SensorQuality::ERROR,
                                             ds18b20_connected));

  // 3. Đọc điện áp ADC cảm biến TDS
  uint16_t raw_1 = 0;
  uint16_t raw_2 = 0;
  const float adc_1 = readTrimmedAdc(PIN_ADC_TDS, raw_1);
  const float adc_2 = readTrimmedAdc(PIN_ADC_TDS, raw_2);
  const float adc_avg = (adc_1 + adc_2) / 2.0F;

  // TẮT NGUỒN GPIO42 ngay sau khi đọc ADC xong để tránh điện phân làm hỏng đầu đo TDS
  digitalWrite(PIN_EN_TDS, HIGH);

  // 4. Tính toán nồng độ TDS có bù nhiệt độ động
  const float v_adc = (adc_avg / SensorConstants::ADC_RESOLUTION_MAX) * SensorConstants::ADC_VREF_VOLTS;
  const float v_sensor = v_adc;
  const float comp_coefficient = 1.0F + SensorConstants::TDS_TEMP_COEFF * (water_temp - SensorConstants::TDS_NEUTRAL_TEMP_C);
  const float comp_voltage = v_sensor / comp_coefficient; // Bù điện áp theo nhiệt độ thực tế
  float tds = (SensorConstants::TDS_POLY_COEFF_A * comp_voltage * comp_voltage * comp_voltage -
               SensorConstants::TDS_POLY_COEFF_B * comp_voltage * comp_voltage +
               SensorConstants::TDS_POLY_COEFF_C * comp_voltage) *
              SensorConstants::TDS_PPM_SCALE_FACTOR;
  if (tds < 0.0F) {
    tds = 0.0F;
  }

  // TDS là tín hiệu analog 0..2,3 V: ADC gần 0 có thể là giá trị hợp lệ
  // ứng với nồng độ rất thấp. Mạch không có chân phát hiện đầu dò, nên
  // không được dùng ngưỡng thấp để kết luận "mất kết nối".
  const bool adc_not_saturated =
      raw_1 < SensorConstants::ADC_RAW_VALID_MAX &&
      raw_2 < SensorConstants::ADC_RAW_VALID_MAX;
  const float larger = adc_1 > adc_2 ? adc_1 : adc_2;
  const float smaller = adc_1 > adc_2 ? adc_2 : adc_1;
  const bool stable = larger <= 1.0F || ((larger - smaller) / larger) <= SensorConstants::TDS_STABILITY_MAX_DIFF_RATIO;
  SensorQuality quality = SensorQuality::GOOD;
  if (!adc_not_saturated) {
    quality = SensorQuality::ERROR;
  } else if (!stable) {
    quality = SensorQuality::UNSTABLE;
  } else if (!ds18b20_connected) {
    quality = SensorQuality::STALE;
  }

  publish(SensorId::TDS,
          makeReading(tds, quality, adc_not_saturated,
                      static_cast<uint16_t>(adc_avg)));
}
} // namespace

// ============================================================================
// CHÍNH: VÒNG LẶP NHIỆM VỤ FREERTOS (TASKSENSORS RUNNING ON CORE 1)
// ============================================================================
void taskSensors(void *parameter) {
  (void)parameter;

  initSensorsHardware(); // Khởi tạo phần cứng các cảm biến

  uint32_t last_level_ms = 0;
  uint32_t last_flow_ms = millis();
  uint32_t last_environment_ms = 0;
  uint32_t last_ph_ms = 0;
  uint32_t last_water_ms = 0;
  uint32_t last_health_ms = 0;

  TickType_t last_wake = xTaskGetTickCount();

  for (;;) {
    const uint32_t now = millis();

    // 1. Đọc phao mực nước mỗi 100ms
    if (now - last_level_ms >= LEVEL_SAMPLE_MS) {
      last_level_ms = now;
      sampleLevels(now);
    }

    // 2. Tính lưu lượng nước mỗi 1000ms
    sampleFlow(now, last_flow_ms);

    // 3. Đọc môi trường không khí (SHT31 & BH1750) mỗi 2000ms
    if (now - last_environment_ms >= ENVIRONMENT_SAMPLE_MS) {
      last_environment_ms = now;
      sampleBh1750();
      sampleSht31();
    }

    // 4. Đọc pH mỗi 2000ms
    if (now - last_ph_ms >= PH_SAMPLE_MS) {
      last_ph_ms = now;
      samplePh();
    }

    // 5. Đọc nhóm nước (DS18B20 & TDS) mỗi 5000ms
    if (now - last_water_ms >= WATER_SAMPLE_MS) {
      last_water_ms = now;
      sampleWaterGroup();
    }

    // 6. Gửi báo cáo sức khỏe Task (Heartbeat) mỗi 1000ms
    if (now - last_health_ms >= 1000) {
      last_health_ms = now;
      app_update_task_health(TaskRole::SENSORS, now, xPortGetCoreID(),
                             uxTaskGetStackHighWaterMark(nullptr));
    }

    // Đủ chu kỳ vTaskDelayUntil (20ms) để nhường CPU cho các Task khác
    vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(SENSOR_TASK_TICK_MS));
  }
}
