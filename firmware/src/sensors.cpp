#include "config.h"
#include <Wire.h>
#include <BH1750.h>

static uint8_t bh1750_addr = 0x23; // Địa chỉ mặc định của cảm biến BH1750
static BH1750 lightMeter;
static bool bh1750_connected = false;
static float last_good_lux = 0.0;
static int bh1750_fail_count = 0;
const int BH1750_MAX_FAILS = 3;
static unsigned long last_query_time = 0;

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
