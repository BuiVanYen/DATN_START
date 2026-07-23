#pragma once // Chỉ include file header này 1 lần duy nhất

// ============================================================================
// HÀM VÒNG LẶP TASK SENSORS FREERTOS (CHẠY ĐỘC LẬP TRÊN CORE 1)
// ============================================================================
// Task này có quyền đọc độc quyền I2C (SHT31/BH1750), OneWire (DS18B20),
// ADC (TDS/pH), chân bật nguồn GPIO42 và xung ngắt cảm biến lưu lượng YF-S201.
void taskSensors(void *parameter);
