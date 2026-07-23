#pragma once // Đảm bảo file header này chỉ được include 1 lần duy nhất trong quá trình biên dịch

#include <Arduino.h> // Nạp các thư viện cơ bản của Arduino ESP32

// ============================================================================
// 1. THÔNG TIN PHIÊN BẢN FIRMWARE VÀ MẠNG MẶC ĐỊNH
// ============================================================================
#define FW_VERSION "2.0.2-manual-rtos" // Chuỗi hiển thị phiên bản firmware hiện tại
#define AP_SSID "DATN_AIOT_LETTUCE"    // Tên Wi-Fi Access Point do ESP32 tự phát ra khi chưa nối Wi-Fi nhà
#define OTA_PORT 80                    // Cổng Cổng HTTP tiêu chuẩn phục vụ Web Dashboard và nạp OTA
#define WS_PORT 81                     // Cổng WebSocket tiêu chuẩn phục vụ truyền dữ liệu Real-time

// ============================================================================
// 2. SƠ ĐỒ CHÂN GPIO CỦA BO MẠCH ESP32-S3-WROOM-1 (N16R8)
// ============================================================================

// --- Nhóm 1: Relay điều khiển thiết bị 220V (Kích mức HIGH 3.3V qua Opto isolator PC817) ---
#define IN_RL2 6 // GPIO6: Điều khiển Máy sục khí oxy 220V (AERATOR_RELAY)
#define IN_RL1 7 // GPIO7: Điều khiển Bơm tuần hoàn chìm 220V (CIRCULATION_RELAY)

// --- Nhóm 2: Tải băm xung PWM điều khiển qua MOSFET (Giá trị từ 0 đến 255) ---
#define DEN1 17   // GPIO17: Đèn LED quang hợp tầng 1 (LIGHT_1)
#define DEN2 18   // GPIO18: Đèn LED quang hợp tầng 2 (LIGHT_2)
#define QUAT1 11  // GPIO11: Quạt thông gió làm mát tầng 1 (FAN_1)
#define QUAT2 10  // GPIO10: Quạt thông gió làm mát tầng 2 (FAN_2)
#define BOMLL1 13 // GPIO13: Bơm định lượng dung dịch dinh dưỡng chai A (PUMP_A)
#define BOMLL2 12 // GPIO12: Bơm định lượng dung dịch dinh dưỡng chai B (PUMP_B)
#define BOMLL3 8  // GPIO8:  Bơm định lượng dung dịch axit hạ pH (PUMP_PH_DOWN)
#define BOM12V 9  // GPIO9:  Bơm 12V DC cấp nước sạch vào bồn chứa (REFILL_PUMP)

// --- Nhóm 3: Chuông báo & Đèn LED chỉ thị hệ thống ---
#define BUZZER 47  // GPIO47: Còi còi cảnh báo khẩn cấp
#define LED_SYS 48 // GPIO48: Đèn LED trạng thái mạng (Nháy nhanh/chậm tùy theo chế độ Wi-Fi)

// --- Nhóm 4: Chân giao tiếp Cảm biến ---
#define PIN_SDA 5     // GPIO5:  Chân I2C Data (nối với BH1750 ánh sáng và SHT31 nhiệt/ẩm)
#define PIN_SCL 4     // GPIO4:  Chân I2C Clock (nối với BH1750 ánh sáng và SHT31 nhiệt/ẩm)
#define PIN_DS18B20 14// GPIO14: Chân dữ liệu OneWire của cảm biến nhiệt độ nước DS18B20
#define PIN_ADC_TDS 1 // GPIO1:  Chân ADC1_CH0 đọc điện áp analog từ cảm biến TDS
#define PIN_EN_TDS 42 // GPIO42: P-MOSF cấp nguồn chung TDS + DS18B20 (Active LOW: LOW=Bật, HIGH=Tắt)
#define PIN_ADC_PH 2  // GPIO2:  Chân ADC1_CH1 đọc điện áp analog từ cảm biến độ pH
#define PIN_FLOW 41   // GPIO41: Chân đếm xung ngắt Interrupt từ cảm biến lưu lượng nước YF-S201
#define PIN_LEVEL1 40 // GPIO40: Phao mực nước bồn chính (Digital Active LOW: LOW=Còn nước, HIGH=Cạn nước)
#define PIN_LEVEL2 39 // GPIO39: Phao chai dung dịch A (Digital Active LOW)
#define PIN_LEVEL3 38 // GPIO38: Phao chai dung dịch B (Digital Active LOW)
#define PIN_LEVEL4 21 // GPIO21: Phao chai dung dịch pH Down (Digital Active LOW)

// ============================================================================
// 3. CẤU HÌNH FREERTOS 2 NHÂN (DUAL CORE CONFIGURATION)
// ============================================================================
constexpr BaseType_t CORE_NETWORK = 0; // Nhân 0: Dành riêng cho xử lý Wi-Fi, WebServer, WebSocket, OTA
constexpr BaseType_t CORE_CONTROL = 1; // Nhân 1: Dành cho Task Safety Control (Nhận lệnh & ghi ngõ ra)
constexpr BaseType_t CORE_SENSORS = 1; // Nhân 1: Dành cho Task Sensors (Đọc 11 cảm biến định kỳ)

// Độ ưu tiên của Task (Số càng lớn thì độ ưu tiên thi hành càng cao)
constexpr UBaseType_t PRIORITY_NETWORK = 2; // Độ ưu tiên Task Network: 2
constexpr UBaseType_t PRIORITY_SENSORS = 3; // Độ ưu tiên Task Sensors: 3
constexpr UBaseType_t PRIORITY_CONTROL = 5; // Độ ưu tiên Task Safety Control: 5 (Cao nhất để luôn ngắt an toàn kịp thời)

// Dung lượng bộ nhớ Stack được cấp phát cho từng Task (đơn vị: Bytes)
constexpr uint32_t STACK_NETWORK = 8192; // Stack 8KB cho Task Mạng (xử lý JSON payload & WebServer)
constexpr uint32_t STACK_SENSORS = 6144; // Stack 6KB cho Task Cảm biến (đọc ADC & thuật toán lọc)
constexpr uint32_t STACK_CONTROL = 8192; // Stack 8KB cho Task Điều khiển An toàn

// ============================================================================
// 4. CHU KỲ LẤY MẪU VÀ THỜI GIAN LEASE AN TOÀN (TIMEOUT CONFIGS)
// ============================================================================
constexpr uint32_t SENSOR_TASK_TICK_MS = 20;    // Tần số đánh thức vòng lặp TaskSensors (mỗi 20ms)
constexpr uint32_t LEVEL_SAMPLE_MS = 100;        // Chu kỳ đọc phao nước: 100ms/lần
constexpr uint32_t LEVEL_DEBOUNCE_MS = 300;      // Thời gian chống nảy phao: 300ms
constexpr uint32_t FLOW_SAMPLE_MS = 1000;        // Chu kỳ tính lưu lượng nước: 1000ms (1 giây)
constexpr uint32_t ENVIRONMENT_SAMPLE_MS = 2000;// Chu kỳ đọc cảm biến không khí SHT31 & BH1750: 2 giây
constexpr uint32_t PH_SAMPLE_MS = 2000;         // Chu kỳ đọc cảm biến pH: 2 giây
constexpr uint32_t WATER_SAMPLE_MS = 5000;      // Chu kỳ đọc cảm biến nhiệt nước DS18B20 & TDS: 5 giây

// Trình duyệt Web gửi tín hiệu Heartbeat mỗi 2 giây. Nếu quá 10 giây không nhận được tín hiệu, ngắt tất cả tải.
constexpr uint32_t MANUAL_WEB_LEASE_MS = 10000;       // Thời gian giữ Lease an toàn Web: 10,000 ms (10 giây)
constexpr uint32_t CONTROL_COMMAND_TTL_MS = 5000;     // Thời gian sống tối đa của 1 lệnh điều khiển trong Queue: 5 giây
constexpr uint32_t WIFI_RECONNECT_TIMEOUT_MS = 15000; // Timeout chờ nối lại STA Wi-Fi trước khi bật SoftAP: 15 giây
constexpr uint32_t STA_RETRY_INTERVAL_MS = 30000;     // Chu kỳ thử kết nối lại Wi-Fi nhà khi đang ở AP Mode: 30 giây
constexpr uint32_t WS_BROADCAST_MS = 1000;            // Chu kỳ broadcast dữ liệu cảm biến qua WebSocket: 1 giây

constexpr uint8_t COMMAND_QUEUE_LENGTH = 24;  // Dung lượng hàng chờ lệnh điều khiển (tối đa 24 mẩu tin)
constexpr uint8_t SENSOR_QUEUE_LENGTH = 32;   // Dung lượng hàng chờ dữ liệu cảm biến (tối đa 32 mẩu tin)
constexpr uint8_t PWM_MIN_RUNNING_VALUE = 25; // Mức PWM tối thiểu để động cơ/bơm bắt đầu quay (xấp xỉ 10%)
