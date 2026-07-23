#pragma once // Chỉ include file header này 1 lần duy nhất

// ============================================================================
// NGƯỠNG THAM CHIẾU CẢM BIẾN TRỒNG XÀ LÁCH THỦY CANH NFT (LETTUCE NFT)
// ============================================================================

// Ngưỡng cường độ ánh sáng phù hợp cho xà lách (Lux)
#define THRESHOLD_LIGHT_LOW  1000.0  // Mức ánh sáng tối thiểu (Dưới 1000 Lux -> Cảnh báo thiếu sáng)
#define THRESHOLD_LIGHT_HIGH 25000.0 // Mức ánh sáng tối đa (Trên 25000 Lux -> Cảnh báo thừa sáng/nắng gắt)

// Ngưỡng nhiệt độ nước dung dịch dinh dưỡng (DS18B20 - °C)
#define THRESHOLD_TEMP_WATER_LOW  18.0 // Nhiệt độ nước tối thiểu cho rễ xà lách
#define THRESHOLD_TEMP_WATER_HIGH 26.0 // Nhiệt độ nước tối đa (Trên 26°C -> Rễ bị thiếu Oxy tan, dễ thối rễ)

// Ngưỡng nhiệt độ không khí (SHT31 - °C)
#define THRESHOLD_TEMP_AIR_LOW  20.0   // Nhiệt độ không khí tối thiểu
#define THRESHOLD_TEMP_AIR_HIGH 30.0   // Nhiệt độ không khí tối đa

// Ngưỡng độ ẩm không khí (SHT31 - %)
#define THRESHOLD_HUMIDITY_LOW  50.0   // Độ ẩm tối thiểu
#define THRESHOLD_HUMIDITY_HIGH 85.0   // Độ ẩm tối đa (Trên 85% -> Dễ phát sinh nấm bệnh)

// Ngưỡng nồng độ dinh dưỡng TDS (Gravity TDS - PPM)
#define THRESHOLD_TDS_LOW  600.0       // Nồng độ PPM tối thiểu cho rau xà lách
#define THRESHOLD_TDS_HIGH 900.0       // Nồng độ PPM tối đa cho rau xà lách

// Ngưỡng độ pH dung dịch
#define THRESHOLD_PH_LOW  5.5          // Độ pH tối thiểu (Dưới 5.5 -> Dung dịch bị Axit hóa)
#define THRESHOLD_PH_HIGH 6.5          // Độ pH tối đa (Trên 6.5 -> Dung dịch bị Kiềm hóa, rễ khó hấp thụ sắt)
