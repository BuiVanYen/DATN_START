#include "sensors.h"
#include "hardware.h"
#include <Wire.h>
#include <BH1750.h>

static BH1750 lightMeter;

void sensors_init() {
    Serial.println("[SENSOR] Dang khoi tao giao tiep I2C va BH1750...");
    
    // Khởi tạo I2C với cấu hình chân custom trên mạch
    Wire.setPins(PIN_SDA, PIN_SCL); // Ghim cứng cấu hình chân vào ESP32 Core
    Wire.begin();
    
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
