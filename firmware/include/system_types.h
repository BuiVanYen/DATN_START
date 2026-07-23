#pragma once // Đảm bảo file header này chỉ được include 1 lần duy nhất

#include <Arduino.h>

// ============================================================================
// 1. ENUM KHAI BÁO CHẾ ĐỘ VẬN HÀNH (SYSTEM MODE)
// ============================================================================
// Chế độ AUTO được định nghĩa sẵn để mở rộng tương lai, nhưng ở phiên bản hiện tại
// firmware chỉ chạy ở chế độ MANUAL (Điều khiển thủ công qua Web Dashboard).
enum class SystemMode : uint8_t {
  MANUAL = 0, // Chế độ thủ công: Người dùng bật/tắt thiết bị qua Web Interface
  AUTO = 1,   // Chế độ tự động: Đang được khóa, dành cho tính năng Cloud IoT về sau
};

// ============================================================================
// 2. ENUM CHẤT LƯỢNG DỮ LIỆU CẢM BIẾN (SENSOR QUALITY)
// ============================================================================
enum class SensorQuality : uint8_t {
  GOOD = 0, // Dữ liệu tốt, mới được lấy mẫu thành công
  STALE,    // Dữ liệu cũ (quá thời gian chu kỳ nhưng chưa lấy mẫu mới)
  ERROR,    // Lỗi phần cứng (Rút dây cảm biến, mất Bus I2C/OneWire, đứt mạch)
  UNSTABLE, // Dữ liệu không ổn định (Nhiễu điện áp ADC quá ngưỡng cho phép)
};

// ============================================================================
// 3. ENUM ĐỊNH DANH 11 CẢM BIẾN TRONG HỆ THỐNG (SENSOR ID)
// ============================================================================
// Đảm bảo không bị trùng lặp ID giữa các cảm biến.
enum class SensorId : uint8_t {
  TEMP_AIR = 0, // 0: Nhiệt độ không khí (°C) từ cảm biến SHT31
  HUMIDITY,     // 1: Độ ẩm không khí (%) từ cảm biến SHT31
  LIGHT,        // 2: Cường độ ánh sáng (Lux) từ cảm biến BH1750
  TEMP_WATER,   // 3: Nhiệt độ nước (°C) từ cảm biến DS18B20
  TDS,          // 4: Nồng độ dinh dưỡng TDS (PPM) từ cảm biến ADC
  PH,           // 5: Độ pH dung dịch từ cảm biến ADC
  FLOW,         // 6: Lưu lượng nước (L/phút) từ cảm biến ngắt xung YF-S201
  LEVEL_MAIN,   // 7: Mực nước bồn chính (0 = Còn nước, 1 = Cạn nước)
  LEVEL_A,      // 8: Mực nước chai dung dịch A (0 = Còn, 1 = Cạn)
  LEVEL_B,      // 9: Mực nước chai dung dịch B (0 = Còn, 1 = Cạn)
  LEVEL_PH,     // 10: Mực nước chai pH Down (0 = Còn, 1 = Cạn)
  COUNT,        // 11: Tổng số lượng cảm biến (Dùng để xác định kích thước mảng)
};

// Ép kiểu hằng số tổng số cảm biến SENSOR_COUNT = 11
constexpr size_t SENSOR_COUNT = static_cast<size_t>(SensorId::COUNT);

// ============================================================================
// 4. ENUM ĐỊNH DANH 10 THIẾT BỊ NGOẠI VI (ACTUATOR ID)
// ============================================================================
enum class ActuatorId : uint8_t {
  CIRCULATION_RELAY = 0, // 0: Relay Bơm tuần hoàn 220V (IN_RL1)
  AERATOR_RELAY,        // 1: Relay Máy sục khí 220V (IN_RL2)
  LIGHT_1,              // 2: PWM Đèn quang hợp tầng 1 (DEN1)
  LIGHT_2,              // 3: PWM Đèn quang hợp tầng 2 (DEN2)
  FAN_1,                // 4: PWM Quạt thông gió tầng 1 (QUAT1)
  FAN_2,                // 5: PWM Quạt thông gió tầng 2 (QUAT2)
  PUMP_A,               // 6: PWM Bơm định lượng chai A (BOMLL1)
  PUMP_B,               // 7: PWM Bơm định lượng chai B (BOMLL2)
  PUMP_PH_DOWN,         // 8: PWM Bơm định lượng pH Down (BOMLL3)
  REFILL_PUMP,          // 9: PWM Bơm cấp nước 12V (BOM12V)
  COUNT,                // 10: Tổng số lượng thiết bị ngoại vi (10 thiết bị)
};

// Ép kiểu hằng số tổng số thiết bị ACTUATOR_COUNT = 10
constexpr size_t ACTUATOR_COUNT = static_cast<size_t>(ActuatorId::COUNT);

// ============================================================================
// 5. ENUM LOẠI LỆNH ĐIỀU KHIỂN (CONTROL COMMAND TYPE)
// ============================================================================
enum class ControlCommandType : uint8_t {
  SET_OUTPUT = 0, // Lệnh cài đặt ngõ ra (Bật/tắt Relay hoặc chỉnh % PWM)
  HEARTBEAT,      // Lệnh nhịp tim gia hạn Lease thời gian sống từ Web Dashboard
  ALL_OFF,        // Lệnh ngắt khẩn cấp tắt toàn bộ 10 thiết bị
  OTA_START,      // Lệnh chuẩn bị nạp OTA (tắt tải an toàn trước khi ghi Flash)
};

// Enum vai trò các Task FreeRTOS để theo dõi sức khỏe thi hành
enum class TaskRole : uint8_t {
  NETWORK = 0, // Task Mạng (Core 0)
  SENSORS,     // Task Cảm biến (Core 1)
  CONTROL,     // Task Điều khiển an toàn (Core 1)
};

// ============================================================================
// 6. CÁC STRUCT DỮ LIỆU CHUẨN HÓA TRONG BỘ NHỚ
// ============================================================================

// Struct chứa 1 kết quả đọc cảm biến hoàn chỉnh
struct SensorReading {
  float value;          // Giá trị đã quy đổi vật lý (°C, %, Lux, PPM, pH, L/min)
  SensorQuality quality;// Đánh giá chất lượng dữ liệu (GOOD, STALE, ERROR, UNSTABLE)
  uint32_t sampled_at_ms;// Thời điểm lấy mẫu (tính theo millis())
  uint16_t raw_value;   // Giá trị điện áp thô đọc từ ADC hoặc xung ngắt
  bool connected;       // Cờ báo cảm biến đang nối phần cứng hay bị đứt
};

// Struct đại diện cho 1 gói tin cập nhật cảm biến truyền qua SensorQueue
struct SensorUpdate {
  SensorId id;           // ID cảm biến nào được cập nhật
  SensorReading reading; // Dữ liệu đọc tương ứng
};

// Struct lưu bản chụp toàn bộ 11 cảm biến trong RAM (được bảo vệ bởi Mutex)
struct SensorSnapshot {
  SensorReading readings[SENSOR_COUNT]; // Mảng lưu kết quả 11 cảm biến
  uint32_t sequence;                   // Số thứ tự bản chụp (tăng dần mỗi lần cập nhật)
};

// Struct lưu bản chụp trạng thái hoạt động của 10 thiết bị ngoại vi trong RAM
struct ActuatorSnapshot {
  int16_t values[ACTUATOR_COUNT]; // Giá trị ngõ ra thực tế của 10 thiết bị (0 hoặc 0-255)
  SystemMode mode;               // Chế độ vận hành (MANUAL)
  bool all_off;                  // Cờ báo toàn bộ tải đang tắt hay bật
  bool web_lease_active;         // Cờ báo Web Lease còn hiệu lực hay đã hết hạn
  uint32_t sequence;             // Số thứ tự bản chụp ngõ ra
  char last_reason[64];          // Chuỗi ký tự lưu lý do chuyển trạng thái gần nhất
};

// Struct đóng gói 1 lệnh điều khiển đẩy vào CommandQueue từ Task Mạng sang Task Safety Control
struct ControlCommand {
  ControlCommandType type; // Loại lệnh (SET_OUTPUT, HEARTBEAT, ALL_OFF, OTA_START)
  ActuatorId actuator;    // Thiết bị nào chịu tác động (nếu là SET_OUTPUT)
  int16_t value;          // Giá trị thiết lập (0-255 hoặc 0-100%)
  uint32_t created_at_ms; // Thời điểm tạo lệnh (millis())
  uint32_t ttl_ms;        // Thời gian sống của lệnh (Time-To-Live, ví dụ 5000ms)
};

// Struct theo dõi sức khỏe hoạt động của 3 Task FreeRTOS (Health Monitor)
struct RuntimeHealth {
  uint32_t network_heartbeat_ms; // Thời điểm nhịp tim gần nhất của Task Network
  uint32_t sensors_heartbeat_ms; // Thời điểm nhịp tim gần nhất của Task Sensors
  uint32_t control_heartbeat_ms; // Thời điểm nhịp tim gần nhất của Task Control
  uint32_t network_stack_words;  // Dung lượng Stack trống nhỏ nhất của Task Network
  uint32_t sensors_stack_words;  // Dung lượng Stack trống nhỏ nhất của Task Sensors
  uint32_t control_stack_words;  // Dung lượng Stack trống nhỏ nhất của Task Control
  int8_t network_core;           // Nhân xử lý thi hành Task Network (Core 0)
  int8_t sensors_core;           // Nhân xử lý thi hành Task Sensors (Core 1)
  int8_t control_core;           // Nhân xử lý thi hành Task Control (Core 1)
};
