#include "config.h"
#include <Wire.h>
#include <BH1750.h>

static BH1750 lightMeter;
static bool bh1750_connected = false;

// Khôi phục I2C bus khi gặp sự cố nhiễu (do đóng ngắt Relay/Bơm tạo tia lửa điện)
void sensors_recover_i2c() {
    Serial.println("[I2C] He thong phat hien treo bus I2C. Dang thuc hien khoi phuc...");
    
    // Cấu hình các chân I2C về chế độ INPUT_PULLUP để giải phóng bus vật lý
    pinMode(PIN_SDA, INPUT_PULLUP);
    pinMode(PIN_SCL, INPUT_PULLUP);
    delay(10);
    
    // Ngắt kết nối Wire và khởi tạo lại
    Wire.end();
    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.setTimeOut(10); // Đặt timeout ngắn (10ms) để không gây nghẽn luồng loop
    
    // Khởi tạo lại cảm biến BH1750
    lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
}

void sensors_init() {
    Serial.println("[SENSOR] Dang khoi tao giao tiep I2C...");
    
    // Khởi tạo I2C với cấu hình chân custom trên mạch
    Wire.setPins(PIN_SDA, PIN_SCL); // Ghim cứng cấu hình chân vào ESP32 Core
    Wire.begin();
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
    
    // Khởi tạo cảm biến BH1750
    if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
        Serial.println("[SENSOR] BH1750 (GY-30) khoi tao thanh cong.");
        bh1750_connected = true;
    } else {
        Serial.println("[SENSOR] CANH BAO: Khong tim thay cam bien BH1750 qua I2C!");
        bh1750_connected = false;
    }
}

float sensors_read_light() {
    // Ping thử thiết bị tại địa chỉ 0x23 (mặc định BH1750) để kiểm tra kết nối vật lý trước khi đọc
    Wire.beginTransmission(0x23);
    byte err = Wire.endTransmission();
    
    if (err != 0) {
        // Thử ping địa chỉ phụ 0x5C nếu chân ADDR bị nối HIGH
        Wire.beginTransmission(0x5C);
        err = Wire.endTransmission();
    }
    
    if (err != 0) {
        if (bh1750_connected) {
            Serial.printf("[SENSOR] BH1750 mat phan hoi (Ma loi: %d). Thuc hien recovery...\n", err);
            sensors_recover_i2c();
            bh1750_connected = false;
        }
        return -1.0;
    }
    
    // Nếu trước đó mất kết nối mà hiện tại ping được -> Khởi tạo lại cảm biến
    if (!bh1750_connected) {
        Serial.println("[SENSOR] BH1750 da ket noi lai. Dang tai khoi tao...");
        if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
            bh1750_connected = true;
        } else {
            return -1.0;
        }
    }
    
    float lux = lightMeter.readLightLevel();
    if (lux < 0) {
        return -1.0;
    }
    return lux;
}

bool sensors_is_bh1750_connected() {
    return bh1750_connected;
}
