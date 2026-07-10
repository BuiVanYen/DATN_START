#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// --- Khởi tạo cảm biến (bao gồm khởi tạo I2C và BH1750) ---
void sensors_init();

// --- Đọc giá trị cường độ ánh sáng từ cảm biến BH1750 (Lux) ---
float sensors_read_light();

#endif // SENSORS_H
