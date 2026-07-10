#include "config.h"
#include <Wire.h>
#include <BH1750.h>

static BH1750 lightMeter;

void sensors_init() {
    Serial.println("[SENSOR] Dang khoi tao giao tiep I2C...");
    
    // Khởi tạo I2C với cấu hình chân custom trên mạch
    Wire.setPins(PIN_SDA, PIN_SCL); // Ghim cứng cấu hình chân vào ESP32 Core
    Wire.begin();
    
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
    } else {
        Serial.println("[SENSOR] CANH BAO: Khong tim thay cam bien BH1750 qua I2C!");
    }
}

float sensors_read_light() {
    float lux = lightMeter.readLightLevel();
    return lux;
}
