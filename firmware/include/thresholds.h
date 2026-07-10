#pragma once

// ============================================================
// NGƯỠNG CẢM BIẾN DINH DƯỠNG & MÔI TRƯỜNG (EcoFarm/Lettuce)
// ============================================================

// Ngưỡng cường độ ánh sáng (Lux)
#define THRESHOLD_LIGHT_LOW  1000.0
#define THRESHOLD_LIGHT_HIGH 25000.0

// Ngưỡng nhiệt độ nước (DS18B20 - độ C)
#define THRESHOLD_TEMP_WATER_LOW  18.0
#define THRESHOLD_TEMP_WATER_HIGH 26.0

// Ngưỡng nhiệt độ không khí (SHT30/SHT31 - độ C)
#define THRESHOLD_TEMP_AIR_LOW  20.0
#define THRESHOLD_TEMP_AIR_HIGH 30.0

// Ngưỡng độ ẩm không khí (SHT30/SHT31 - %)
#define THRESHOLD_HUMIDITY_LOW  50.0
#define THRESHOLD_HUMIDITY_HIGH 85.0

// Ngưỡng nồng độ dinh dưỡng TDS (ppm)
#define THRESHOLD_TDS_LOW  600.0
#define THRESHOLD_TDS_HIGH 900.0

// Ngưỡng độ pH
#define THRESHOLD_PH_LOW  5.5
#define THRESHOLD_PH_HIGH 6.5
