#include "config.h"
#include <Wire.h>
#include <BH1750.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_SHT31.h>

static uint8_t bh1750_addr = 0x23; // Địa chỉ mặc định của cảm biến BH1750
static BH1750 lightMeter;
static bool bh1750_connected = false;
static float last_good_lux = 0.0;
static int bh1750_fail_count = 0;
const int BH1750_MAX_FAILS = 3;
static unsigned long last_query_time = 0;

static OneWire oneWire(PIN_DS18B20);
static DallasTemperature tempSensor(&oneWire);
static Adafruit_SHT31 sht31;
static bool sht31_connected = false;

// Biến cho cảm biến lưu lượng nước
static volatile uint32_t flow_pulse_count = 0;
static unsigned long last_flow_time = 0;
static float current_flow_rate = 0.0;

void IRAM_ATTR flow_pulse_counter() {
    flow_pulse_count++;
}

// Khôi phục I2C bus khi gặp sự cố nhiễu (do đóng ngắt Relay/Bơm tạo tia lửa điện)
void sensors_recover_i2c() {
    Serial.println("[I2C] He thong phat hien treo bus I2C. Dang thuc hien khoi phuc...");
    
    // 1. Tắt driver Wire để can thiệp GPIO trực tiếp
    Wire.end();
    
    // 2. Cấu hình SCL làm OUTPUT, SDA làm INPUT để kiểm tra và kích xung nhịp
    pinMode(PIN_SDA, INPUT);
    pinMode(PIN_SCL, OUTPUT);
    
    // 3. Nếu SDA bị kéo thấp (slave giữ bus), gửi chuỗi xung nhịp SCL để giải phóng
    for (int i = 0; i < 9; i++) {
        if (digitalRead(PIN_SDA) == HIGH) {
            Serial.printf("[I2C] Bus da duoc giai phong o xung nhip thu %d\n", i);
            break;
        }
        digitalWrite(PIN_SCL, LOW);
        delayMicroseconds(5);
        digitalWrite(PIN_SCL, HIGH);
        delayMicroseconds(5);
    }
    
    // 4. Khởi tạo lại Wire bus
    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.setClock(400000); // Thiết lập tần số 400kHz (Fast Mode)
    Wire.setTimeOut(10); // Đặt timeout ngắn (10ms) để tránh nghẽn luồng loop
    
    // 5. Cấu hình lại cảm biến BH1750
    lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, bh1750_addr);
}

void sensors_init() {
    Serial.println("[SENSOR] Dang khoi tao giao tiep I2C...");

    // Khoi tao cac chan cam bien muc nuoc
    pinMode(PIN_LEVEL1, INPUT);
    pinMode(PIN_LEVEL2, INPUT);
    pinMode(PIN_LEVEL3, INPUT);
    pinMode(PIN_LEVEL4, INPUT);
    
    // Khởi tạo I2C trực tiếp với các chân custom
    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.setClock(400000); // Thiết lập tần số 400kHz (Fast Mode)
    Wire.setTimeOut(10);            // Thiết lập I2C timeout là 10ms để chống nghẽn
    
    // --- QUÉT THIẾT BỊ I2C ---
    Serial.println("[I2C] Bat dau quet thiet bi tren bus I2C...");
    byte error, address;
    int nDevices = 0;
    
    for(address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.printf("[I2C] Tim thay thiet bi tai dia chi: 0x%02X\n", address);
            if (address == 0x23 || address == 0x5C) {
                bh1750_addr = address;
                Serial.printf("[I2C] Tu dong nhận dạng BH1750 tại địa chỉ: 0x%02X\n", bh1750_addr);
            }
            nDevices++;
        } else if (error == 4) {
            Serial.printf("[I2C] Loi khong xac dinh tai dia chi: 0x%02X\n", address);
        }
    }
    if (nDevices == 0) {
        Serial.println("[I2C] Khong tim thay thiet bi I2C nao!");
    } else {
        Serial.printf("[I2C] Quet xong. Tim thay %d thiet bi.\n\n", nDevices);
    }
    
    // Khởi tạo cảm biến BH1750 với địa chỉ phát hiện được
    if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, bh1750_addr)) {
        Serial.printf("[SENSOR] BH1750 (GY-30) khoi tao thanh cong tai dia chi 0x%02X.\n", bh1750_addr);
        bh1750_connected = true;
    } else {
        Serial.printf("[SENSOR] CANH BAO: Khong the khoi tao BH1750 tai dia chi 0x%02X!\n", bh1750_addr);
        bh1750_connected = false;
    }

    // Khởi tạo cảm biến DS18B20
    tempSensor.begin();
    Serial.println("[SENSOR] DS18B20 khoi tao xong.");

    // Khởi tạo cảm biến SHT31
    if (sht31.begin(0x44)) {
        Serial.println("[SENSOR] SHT31 khoi tao thanh cong tai dia chi 0x44.");
        sht31_connected = true;
    } else {
        Serial.println("[SENSOR] CANH BAO: Khong the khoi tao SHT31 tai dia chi 0x44!");
        sht31_connected = false;
    }

    // Cấu hình chân điều khiển nguồn TDS (P-MOSFET)
    pinMode(PIN_EN_TDS, OUTPUT);
    digitalWrite(PIN_EN_TDS, HIGH); // Tắt nguồn ban đầu (EN_TDS HIGH để tắt P-MOSFET)

    // Khởi tạo cảm biến lưu lượng nước
    pinMode(PIN_FLOW, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_FLOW), flow_pulse_counter, FALLING);
    last_flow_time = millis();
    Serial.println("[SENSOR] Cam bien luu luong nuoc khoi tao xong.");
}

float sensors_read_light() {
    unsigned long now = millis();

    // 1. Rate-limiting: Giới hạn tần suất đọc cảm biến (tối thiểu 2 giây một lần)
    // Bỏ qua rate limit ở lần đọc đầu tiên (last_query_time == 0)
    if (last_query_time != 0 && (now - last_query_time < 2000)) {
        return last_good_lux;
    }
    last_query_time = now;

    // 2. Ping thử thiết bị tại địa chỉ hiện tại
    Wire.beginTransmission(bh1750_addr);
    byte err = Wire.endTransmission();
    
    // Nếu ping lỗi, quét thử địa chỉ thay thế (phòng trường hợp chân ADDR của BH1750 bị dao động điện áp)
    if (err != 0) {
        uint8_t alt_addr = (bh1750_addr == 0x23) ? 0x5C : 0x23;
        Wire.beginTransmission(alt_addr);
        if (Wire.endTransmission() == 0) {
            bh1750_addr = alt_addr;
            // Tái khởi tạo lightMeter với địa chỉ mới
            if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, bh1750_addr)) {
                err = 0;
                bh1750_connected = true;
                bh1750_fail_count = 0;
                Serial.printf("[SENSOR] BH1750 tu dong chuyen sang dia chi moi: 0x%02X\n", bh1750_addr);
            } else {
                Serial.printf("[SENSOR] LOI: Khong the tai khoi tao BH1750 tai dia chi moi 0x%02X\n", bh1750_addr);
            }
        }
    }
    
    if (err != 0) {
        bh1750_fail_count++;
        Serial.printf("[SENSOR] BH1750 ping that bai (%d/%d), Ma loi: %d\n", bh1750_fail_count, BH1750_MAX_FAILS, err);

        if (bh1750_fail_count >= BH1750_MAX_FAILS) {
            Serial.println("[SENSOR] BH1750 mat ket noi hoan toan. Thuc hien recovery I2C...");
            sensors_recover_i2c();
            bh1750_fail_count = 0; // Reset để thử lại

            // Kiểm tra xem recovery có thành công không
            Wire.beginTransmission(bh1750_addr);
            if (Wire.endTransmission() == 0) {
                bh1750_connected = true;
                Serial.println("[SENSOR] Recovery I2C thanh cong! Thu doc lai cam bien...");
                delay(120); // Chờ cảm biến ổn định
                // Không return, để tiếp tục đọc cảm biến bên dưới
            } else {
                bh1750_connected = false;
                return -1.0;
            }
        } else {
            return last_good_lux;
        }
    }
    
    // Nếu trước đó mất kết nối mà hiện tại ping được -> Tái khởi tạo cảm biến
    if (!bh1750_connected) {
        Serial.println("[SENSOR] BH1750 bat ngo ping duoc lai. Dang tai khoi tao...");
        if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, bh1750_addr)) {
            bh1750_connected = true;
            bh1750_fail_count = 0;
            delay(120); // Chờ cảm biến thực hiện xong chu kỳ đo đầu tiên
        } else {
            bh1750_fail_count++;
            return last_good_lux;
        }
    }
    
    // Định kỳ gửi lệnh cấu hình lại chế độ đo (mỗi 5 lần đọc thành công)
    // Để đánh thức cảm biến nếu chip bị reset ngầm về chế độ Power Down do sụt áp
    // Lưu ý: Do hàm này có rate-limit 2 giây, 5 lần đọc ≈ 10 giây
    static int reconfig_counter = 0;
    reconfig_counter++;
    if (reconfig_counter >= 5) {
        reconfig_counter = 0;
        lightMeter.configure(BH1750::CONTINUOUS_HIGH_RES_MODE);
    }
    
    float lux = lightMeter.readLightLevel();
    if (lux < 0) {
        bh1750_fail_count++;
        Serial.printf("[SENSOR] BH1750 readLightLevel tra ve loi (%d/%d), Lux = %.2f\n", bh1750_fail_count, BH1750_MAX_FAILS, lux);
        if (bh1750_fail_count >= BH1750_MAX_FAILS) {
            bh1750_connected = false;
            return -1.0;
        }
        return last_good_lux;
    }
    
    // Đọc thành công -> Reset bộ đếm lỗi và lưu lại giá trị tốt nhất
    bh1750_fail_count = 0;
    bh1750_connected = true;
    last_good_lux = lux;
    return lux;
}

bool sensors_is_bh1750_connected() {
    return bh1750_connected;
}

// Cảm biến mực nước (Active LOW):
// - Phát hiện nước/vật chạm → Output kéo LOW (0V) → ESP32 đọc LOW → 1.0 (Còn nước)
// - Không có nước/không chạm → Output giữ HIGH (5V) → ESP32 đọc HIGH → 0.0 (Hết nước)
float sensors_read_level1() {
    return (digitalRead(PIN_LEVEL1) == LOW) ? 1.0 : 0.0;
}

float sensors_read_level2() {
    return (digitalRead(PIN_LEVEL2) == LOW) ? 1.0 : 0.0;
}

float sensors_read_level3() {
    return (digitalRead(PIN_LEVEL3) == LOW) ? 1.0 : 0.0;
}

float sensors_read_level4() {
    return (digitalRead(PIN_LEVEL4) == LOW) ? 1.0 : 0.0;
}

// --- Cảm biến nhiệt độ & độ ẩm không khí SHT31 ---
float sensors_read_temp_air() {
    if (!sht31_connected) return -127.0;
    float temp = sht31.readTemperature();
    if (isnan(temp)) return -127.0;
    return temp;
}

float sensors_read_humidity() {
    if (!sht31_connected) return -1.0;
    float hum = sht31.readHumidity();
    if (isnan(hum)) return -1.0;
    return hum;
}

bool sensors_is_sht31_connected() {
    return sht31_connected;
}

// --- Cảm biến nhiệt độ nước DS18B20 ---
static bool ds18b20_connected = false;

float sensors_read_temp_water() {
    // 1. Cấp nguồn cho mô-đun TDS & Cảm biến (Q8 P-MOSFET dẫn khi chân EN_TDS kéo LOW)
    digitalWrite(PIN_EN_TDS, LOW);
    delay(50); // Chờ 50ms để cảm biến khởi động ổn định nguồn
    
    // 2. Ép quét lại bus 1-Wire để tìm cảm biến sau khi được cấp nguồn
    tempSensor.begin();
    
    // 3. Yêu cầu cảm biến đo và lấy giá trị
    tempSensor.requestTemperatures();
    float t = tempSensor.getTempCByIndex(0);
    
    // 4. Cập nhật trạng thái kết nối và trả về giá trị
    if (t == DEVICE_DISCONNECTED_C || t == -127.0) {
        ds18b20_connected = false;
        t = -127.0;
    } else {
        ds18b20_connected = true;
    }
    
    // Lưu ý: Không tắt nguồn ở đây vì hàm sensors_read_tds() chạy tiếp ngay sau đó sẽ cần nguồn.
    // Nguồn của mô-đun sẽ được ngắt tự động ở cuối hàm sensors_read_tds() để tránh điện phân.
    return t;
}

bool sensors_is_ds18b20_connected() {
    return ds18b20_connected;
}

// --- Cảm biến dinh dưỡng TDS (Đo bằng xung nguồn P-MOSFET bảo vệ) ---
float sensors_read_tds(float water_temp) {
    // 1. Bật nguồn TDS (kéo LOW EN_TDS)
    digitalWrite(PIN_EN_TDS, LOW);
    delay(100); // Ổn định nguồn
    
    // 2. Lấy 30 mẫu ADC
    const int NUM_SAMPLES = 30;
    int samples[NUM_SAMPLES];
    for (int i = 0; i < NUM_SAMPLES; i++) {
        samples[i] = analogRead(PIN_ADC_TDS);
        delay(5);
    }
    
    // 3. Tắt nguồn TDS (kéo HIGH EN_TDS) để tránh điện phân
    digitalWrite(PIN_EN_TDS, HIGH);
    
    // 4. Sắp xếp mảng bằng Insertion Sort để lọc nhiễu
    for (int i = 1; i < NUM_SAMPLES; i++) {
        int key = samples[i];
        int j = i - 1;
        while (j >= 0 && samples[j] > key) {
            samples[j + 1] = samples[j];
            j = j - 1;
        }
        samples[j + 1] = key;
    }
    
    // 5. Tính trung bình 60% mẫu ở giữa (loại bỏ 20% cao nhất và 20% thấp nhất)
    long sum = 0;
    for (int i = 6; i < NUM_SAMPLES - 6; i++) {
        sum += samples[i];
    }
    float avg_adc = (float)sum / 18.0;
    
    // 6. Quy đổi ra điện áp chân ESP32 (V)
    float v_adc = (avg_adc / 4095.0) * 3.3;
    
    // 7. Bù điện áp do cầu phân áp (1.0 cho breadboard test, 1.6667 cho PCB chính thức)
    float v_sensor = v_adc * 1.0;
    
    // 8. Bù nhiệt độ
    if (water_temp <= 0.0 || water_temp > 50.0) {
        water_temp = 25.0; // Mặc định 25 độ
    }
    float compCoefficient = 1.0 + 0.02 * (water_temp - 25.0);
    float compVoltage = v_sensor / compCoefficient;
    
    // 9. Tính toán TDS ppm từ điện áp
    float tds = (133.42 * compVoltage * compVoltage * compVoltage 
                - 255.86 * compVoltage * compVoltage 
                + 857.39 * compVoltage) * 0.5;
                
    Serial.printf("[TDS] ADC_Avg: %.1f | V_ADC: %.3fV | V_Sensor: %.3fV | Temp: %.1fC | TDS: %.1f ppm\n", 
                  avg_adc, v_adc, v_sensor, water_temp, tds);
                  
    return (tds < 0.0) ? 0.0 : tds;
}

bool sensors_is_tds_connected() {
    return true; // Tín hiệu analog
}

// --- Cảm biến pH ---
float sensors_read_ph() {
    long sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += analogRead(PIN_ADC_PH);
        delay(10);
    }
    float avg_adc = (float)sum / 10.0;
    float v_adc = (avg_adc / 4095.0) * 3.3;
    
    // Hệ số cầu phân áp: 1.0 cho breadboard, 1.6667 cho PCB (ở đây dùng PCB chính thức)
    float v_sensor = v_adc * 1.6667; 
    
    // Công thức tính pH từ điện áp (mẫu: pH 7 = 2.5V, độ dốc 0.18V/pH)
    float ph = 7.0 + ((2.5 - v_sensor) / 0.18);
    if (ph < 0.0) ph = 0.0;
    if (ph > 14.0) ph = 14.0;
    
    return ph;
}

bool sensors_is_ph_connected() {
    return true;
}

// --- Cảm biến lưu lượng nước ---
float sensors_read_flow() {
    unsigned long now = millis();
    unsigned long duration = now - last_flow_time;
    if (duration >= 1000) {
        uint32_t pulses = flow_pulse_count;
        flow_pulse_count = 0;
        last_flow_time = now;
        
        float hz = (float)pulses / (duration / 1000.0);
        // Hz = 7.5 * Q (L/m) => Q = Hz / 7.5
        current_flow_rate = hz / 7.5;
    }
    return current_flow_rate;
}

bool sensors_is_flow_connected() {
    return true;
}
