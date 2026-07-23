# Nhật Ký Vận Hành Firmware EcoFarm RTOS (Phiên Bản Tối Ưu)

*(Bản giải thích cho người mới học C/C++ & FreeRTOS nâng cao kiến thức và hiểu rõ firmware)*

---

## 1. Firmware này giải quyết việc gì?

Firmware được tổ chức để tách biệt 3 nhóm nhiệm vụ chính:

1. **Đọc cảm biến (`TaskSensors`)**
2. **Điều khiển thiết bị ngoại vi & An toàn (`TaskSafetyControl`)**
3. **Xử lý Wi-Fi / Web Server / WebSocket / OTA (`TaskNetwork`)**

Phiên bản cũ để tất cả trong một vòng `loop()` lớn. Khi ứng dụng phình to, cách làm này gây block hệ thống, khiến giao diện web bị giật và mất an toàn khi truyền dữ liệu. Firmware này chuyển toàn bộ sang **FreeRTOS 2 nhân (Core 0 và Core 1)** của ESP32-S3.

---

## 2. Cấu trúc thư mục firmware sau khi tối ưu & tách nhỏ

```text
firmware/
├── include/
│   ├── config.h           # Cấu hình chân GPIO, tần số PWM, chu kỳ đọc & timeout
│   ├── system_types.h     # Định nghĩa enum class, struct dữ liệu trung tâm
│   ├── rtos_app.h         # Giao diện Queue, Mutex và Snapshots bộ nhớ dùng chung
│   ├── sensors.h          # Header nhiệm vụ đọc cảm biến
│   ├── actuator.h         # Header điều khiển GPIO / PWM phần cứng an toàn
│   ├── failsafe.h         # Header nhiệm vụ kiểm soát an toàn & điều khiển MANUAL
│   ├── wifi_manager.h     # Quản lý kết nối Wi-Fi AP/STA, NVS Preferences & Captive Portal
│   ├── web_api.h          # HTTP Endpoints, WebSocket server & tạo JSON System Status
│   ├── ota_update.h       # Xử lý cập nhật nạp code từ xa qua Web OTA (Arduino Update)
│   ├── network_task.h     # Task FreeRTOS chạy vòng lặp mạng trên Core 0
│   ├── ota_handler.h      # Wrapper tương thích ngược trỏ về network_task
│   ├── mqtt_handler.h     # Header stub cho MQTT (Đã đánh dấu FUTURE - NOT USED)
│   ├── thresholds.h       # Ngưỡng cảnh báo cảm biến
│   └── web_assets.h       # Gzip compressed byte array (HTML/CSS/JS) tự động sinh
├── src/
│   ├── main.cpp           # Khởi động an toàn, khởi tạo RTOS và tạo 3 Tasks
│   ├── rtos_app.cpp       # Triển khai Queue, Mutex, lưu trữ Snapshots & Task Health
│   ├── sensors.cpp        # Đọc 11 giá trị cảm biến với hằng số chuẩn hóa
│   ├── actuator.cpp       # Ánh xạ bảng thiết bị, ghi băm xung PWM / Relay an toàn
│   ├── failsafe.cpp       # TaskSafetyControl nhận lệnh MANUAL & duy trì Lease timeout
│   ├── wifi_manager.cpp   # Khởi tạo AP/STA, lưu mật khẩu NVS, DNS server
│   ├── web_api.cpp        # Route WebServer HTTP, gửi nhận WebSocket JSON payload
│   ├── ota_update.cpp     # Upload handler cập nhật firmware
│   ├── network_task.cpp   # Vòng lặp điều phối mạng, nhấp nháy LED trạng thái
│   └── mqtt_handler.cpp   # Code để dành cho tính năng Cloud MQTT sau này
├── web/
│   ├── index.html         # Giao diện Web Dashboard (Dùng data-actuator thay cho pin)
│   ├── style.css          # CSS thiết kế giao diện hiện đại
│   └── script.js          # Logic điều khiển, WebSocket client (Không phụ thuộc pin GPIO)
├── platformio.ini         # Cấu hình PlatformIO & thư viện
└── generate_web_assets.py # Script Python nén Gzip web assets tự động & deterministic
```

### Nhiệm vụ của từng nhóm file

- `main.cpp`: Khởi động phần cứng an toàn, tạo 3 task FreeRTOS.
- `rtos_app.cpp`: Quản lý các tài nguyên chia sẻ an toàn (Queue, Mutex, Snapshot, Health check).
- `sensors.cpp`: Đọc định kỳ 11 tham số cảm biến, áp dụng lọc nhiễu ADC và tính toán theo các hằng số vật lý có tên gọi rõ ràng.
- `actuator.cpp`: Ghi trực tiếp lên chân phần cứng GPIO/PWM, chuyển đổi `ActuatorId` thành chân vật lý.
- `failsafe.cpp`: Task điều khiển trung tâm (`TaskSafetyControl`). Duy trì Web Lease, ngắt toàn bộ tải khi mất kết nối mạng.
- `wifi_manager.cpp`: Đảm bảo Wi-Fi kết nối STA hoặc tự bật AP (Captive Portal `192.168.4.1` / `datn.local`).
- `web_api.cpp`: Định nghĩa các đường dẫn HTTP REST API & xử lý dữ liệu qua WebSocket.
- `ota_update.cpp`: Cập nhật firmware trực tiếp trên trình duyệt mà không cần dây nạp.
- `network_task.cpp`: Task chính chạy trên **Core 0** duy trì việc lắng nghe mạng.

---

## 3. Sơ đồ 2 nhân của ESP32-S3

### Core 0
Chạy `TaskNetwork` (Priority 2 - `PRIORITY_NETWORK`):
- Quản lý Wi-Fi STA / SoftAP
- Captive Portal DNS Server
- WebServer HTTP (`/`, `/api/status`, `/api/control`, `/api/wifi`, `/update`)
- WebSockets Server (Broadcast dữ liệu định kỳ & nhận lệnh điều khiển)
- Xử lý Web OTA Update

### Core 1
Chạy 2 Tasks:
- `TaskSensors` (Priority 3 - `PRIORITY_SENSORS`): Đọc định kỳ cảm biến (Phao, Flow, Temp Air/Water, Humidity, Light, pH, TDS)
- `TaskSafetyControl` (Priority 5 - `PRIORITY_CONTROL`): Nhận lệnh từ `CommandQueue`, áp dụng trạng thái ngoại vi, kiểm tra Web Lease.

### Vì sao phải tách 2 nhân như vậy?
Nếu kết nối Wi-Fi bị chập chờn, client web reload liên tục hoặc đang nạp OTA dung lượng lớn, việc đọc cảm biến và duy trì timer an toàn cho máy bơm/relay vẫn diễn ra hoàn toàn độc lập ở Core 1.

---

## 4. Luồng dữ liệu & Điều khiển an toàn

```text
Web UI (Browser) / HTTP / WebSocket
        │ (Gửi lệnh dạng Actuator ID string, ví dụ: "DEN1", "BOMLL1")
        ▼
TaskNetwork (Core 0)
        │ (Đóng gói thành ControlCommand đẩy vào Queue)
        ▼
app_send_control_command() -> CommandQueue
        │
        ▼
TaskSafetyControl (Core 1)
        │ (Kiểm tra TTL, ghi nhận Web Lease, gửi giá trị tới actuator_apply)
        ▼
GPIO / PWM phần cứng thực tế (chuyển qua bảng ánh xạ actuator.cpp)
```

Dữ liệu cảm biến đi theo luồng ngược lại:

```text
TaskSensors (Core 1)
        │
        ▼
app_publish_sensor() -> SensorSnapshot (Bộ nhớ RAM dùng chung được bảo vệ bởi Mutex)
        │
        ├────────► TaskSafetyControl (Lấy dữ liệu từ SensorQueue để giải phóng Queue)
        │
        └────────► TaskNetwork (Đóng gói thành JSON gửi về Web Dashboard)
```

> **Đặc biệt:** `TaskNetwork` **không được phép ghi GPIO trực tiếp**. Mọi thao tác bật/tắt thiết bị bắt buộc phải đi qua `CommandQueue` tới `TaskSafetyControl`.

---

## 5. Chế độ vận hành hiện tại (MANUAL Mode)

Firmware hiện tại tập trung cho chế độ **MANUAL**:

- Khởi động mặc định ở chế độ `MANUAL`.
- Tất cả 10 thiết bị ngoại vi đều ở trạng thái **OFF** khi vừa cấp nguồn, sau khi reset hoặc sau khi nạp OTA xong.
- Mã nguồn giữ các cấu trúc `SystemMode::AUTO` và `SensorQueue` để chuẩn bị cho việc mở rộng về sau. Các phần này được đánh dấu `// FUTURE - NOT USED IN MANUAL MODE`.

---

## 6. Danh sách 10 thiết bị ngoại vi được quản lý

### Relay (Bật/Tắt)
- `IN_RL1` (GPIO7): Bơm tuần hoàn 220V (`CIRCULATION_RELAY`)
- `IN_RL2` (GPIO6): Máy sục khí 220V (`AERATOR_RELAY`)

### PWM (Độ sáng / Tốc độ 0 - 255)
- `DEN1` (GPIO17): Đèn quang hợp tầng 1 (`LIGHT_1`)
- `DEN2` (GPIO18): Đèn quang hợp tầng 2 (`LIGHT_2`)
- `QUAT1` (GPIO11): Quạt thông gió tầng 1 (`FAN_1`)
- `QUAT2` (GPIO10): Quạt thông gió tầng 2 (`FAN_2`)
- `BOMLL1` (GPIO13): Bơm định lượng dung dịch A (`PUMP_A`)
- `BOMLL2` (GPIO12): Bơm định lượng dung dịch B (`PUMP_B`)
- `BOMLL3` (GPIO8): Bơm định lượng dung dịch pH Down (`PUMP_PH_DOWN`)
- `BOM12V` (GPIO9): Bơm cấp nước sạch vào bình chứa (`REFILL_PUMP`)

### Nguyên tắc an toàn (Fail-Safe)
- Mất kết nối Web (Web Lease Timeout > 10 giây) ➔ Tự động tắt tất cả 10 thiết bị.
- Mất Wi-Fi ➔ Tắt tất cả thiết bị.
- Đang bắt đầu OTA ➔ Tắt tất cả thiết bị trước khi ghi Flash.

---

## 7. 11 giá trị cảm biến được đo đạc

1. **Nhiệt độ không khí (°C)** - SHT31
2. **Độ ẩm không khí (%)** - SHT31
3. **Ánh sáng (Lux)** - BH1750 (Auto-recover bus I2C)
4. **Nhiệt độ nước (°C)** - DS18B20 (Active power bus)
5. **Dung dịch TDS (PPM)** - Analog ADC (Bù nhiệt độ động)
6. **Độ pH** - Analog ADC (Lọc nhiễu ADC đa mẫu)
7. **Lưu lượng nước (L/phút)** - Cảm biến Flow YF-S201 (Xung Interrupt)
8. **Phao bồn chính** - Digital Active LOW (Debounced)
9. **Phao chai A** - Digital Active LOW
10. **Phao chai B** - Digital Active LOW
11. **Phao chai pH Down** - Digital Active LOW

---

## 8. Chân điều khiển cấp nguồn cảm biến TDS (`PIN_EN_TDS` - GPIO42)

`GPIO42` là chân cấp nguồn riêng cho mạch đo TDS/Nhiệt độ nước (Active LOW):
- `LOW`: Cấp nguồn cho mạch đo.
- `HIGH`: Ngắt nguồn hoàn toàn.

Mục đích: Tránh hiện tượng điện phân cực làm hỏng đầu đo TDS khi ngâm liên tục trong dung dịch thủy canh. `TaskSensors` chỉ bật `GPIO42` trong khoảng vài trăm miligiây để lấy mẫu ADC rồi ngắt ngay lập tức.

---

## 9. Loại bỏ việc khai báo lặp chân GPIO trên Web Interface

Ở bản thiết kế trước, các chân GPIO (như 17, 18, 6, 7...) bị khai báo trùng lặp ở 3 nơi (`config.h`, `index.html`, `script.js`).

Ở phiên bản mới:
- File `index.html` và `script.js` **không chứa bất kỳ số GPIO nào**.
- Các thẻ HTML dùng thuộc tính tên thiết bị: `data-actuator="DEN1"`, `data-actuator="IN_RL1"`, v.v.
- JavaScript gửi request về server dạng JSON: `{"actuator": "DEN1", "state": 100}`.
- Firmware nhận tên mã `"DEN1"` và hàm `actuator_name_to_id()` trong `actuator.cpp` sẽ ánh xạ sang enum `ActuatorId::LIGHT_1`, sau đó tra cứu chân GPIO17 duy nhất được định nghĩa trong `config.h`.

---

## 10. Dữ liệu trạng thái API JSON (`age` & `quality`)

Hệ thống tính toán 2 thuộc tính quan trọng cho từng cảm biến trong JSON API (`/api/status` và WebSocket):
- `quality`: Trạng thái dữ liệu (`GOOD`, `STALE`, `UNSTABLE`, `ERROR`).
- `age`: Thời gian kể từ lần lấy mẫu gần nhất (tính bằng miligiây).

Hai trường này giúp các ứng dụng mở rộng (Mobile App, Python Monitor script hoặc MQTT Gateway) đánh giá được độ tin cậy của dữ liệu. Giao diện Web đơn giản hóa tập trung hiển thị giá trị đo và biểu tượng kết nối.

---

## 11. Các khái niệm C++ & FreeRTOS dành cho người mới học

Để tự đọc hiểu toàn bộ mã nguồn của dự án này, dưới đây là giải thích ngắn gọn các khái niệm C++ nâng cao và FreeRTOS được sử dụng trong codebase:

### 1. `enum class` (Strongly Typed Enumeration)
Khác với `enum` truyền thống của C (dễ bị trùng tên hằng số và tự động ép kiểu sang `int`), `enum class` trong C++ bắt buộc phải truy cập qua tên Scope (ví dụ: `ActuatorId::PUMP_A`).
```cpp
enum class ActuatorId : uint8_t {
  CIRCULATION_RELAY = 0,
  LIGHT_1,
  PUMP_A,
  COUNT
};
```
Điều này giúp biên dịch an toàn, không thể nhầm lẫn giữa ID của cảm biến và ID của thiết bị ngoại vi.

### 2. `namespace` (Không gian tên)
Được dùng để nhóm các biến và hàm nội bộ, tránh xung đột tên hàm giữa các file `.cpp` khác nhau.
```cpp
namespace {
  // Biến hoặc hàm nằm trong anonymous namespace
  // chỉ có hiệu lực bên trong file cpp này.
  uint32_t local_timer = 0;
}
```

### 3. Tham chiếu `&` (Reference)
Trong C++, dấu `&` khi khai báo tham số hàm (như `void update(const SensorSnapshot &snapshot)`) giúp truyền dữ liệu trực tiếp bằng địa chỉ bộ nhớ mà không cần copy toàn bộ struct lớn, giúp giảm dung lượng RAM và tăng tốc độ xử lý mà cú pháp vẫn sạch hơn dùng con trỏ `*`.

### 4. `constexpr` & `static_cast`
- `constexpr`: Báo cho trình biên dịch tính toán hằng số ngay từ lúc biên dịch (Compile-time), tiết kiệm thời gian chạy của vi xử lý.
- `static_cast<Type>(value)`: Ép kiểu dữ liệu C++ an toàn (thay thế cho kiểu ép C-style `(int)val`). Ví dụ: `static_cast<size_t>(SensorId::PH)`.

### 5. FreeRTOS: Queue, Mutex, Task, Core, Priority
- **Queue (`QueueHandle_t`)**: Hàng chờ dữ liệu an toàn đa luồng. Một Task đẩy lệnh vào Queue, Task khác lấy lệnh ra xử lý mà không bị ghi đè dữ liệu.
- **Mutex (`SemaphoreHandle_t`)**: Khóa bộ nhớ. Khi một Task đang đọc/ghi struct `SensorSnapshot`, nó sẽ khoá Mutex lại để Task khác không ghi đè dở dang (ngăn ngừa tranh chấp tài nguyên - Race Condition).
- **Task**: Một tiến trình (Thread) chạy độc lập có hàm vòng lặp vô hạn `for (;;)` riêng.
- **Core (0 hoặc 1)**: Nhân xử lý của ESP32-S3 được chỉ định khi gọi `xTaskCreatePinnedToCore()`.
- **Priority**: Độ ưu tiên thực thi của Task trong `config.h`: `PRIORITY_NETWORK` (2), `PRIORITY_SENSORS` (3), `PRIORITY_CONTROL` (5).

### 6. Mẫu thiết kế `Snapshot` & `Heartbeat` Lease
- **Snapshot**: Lưu lại một bản chụp trạng thái toàn cục của hệ thống tại một thời điểm. Nhờ đó Task Mạng có thể lấy dữ liệu đọc sensor mà không cần trực tiếp truy cập vào phần cứng cảm biến.
- **Heartbeat Lease**: Cơ chế đếm ngược an toàn. Trình duyệt Web gửi tín hiệu "Heartbeat" định kỳ 2 giây. Nếu sau 10 giây (`MANUAL_WEB_LEASE_MS`) firmware không nhận được tín hiệu nào, hệ thống kết luận Web đã bị đóng/rớt mạng và kích hoạt Fail-safe ngắt toàn bộ thiết bị.

### 7. Callback và Con Trỏ `void *`
- **Callback**: Một hàm được truyền làm tham số để hệ thống gọi lại khi có sự kiện xảy ra (ví dụ: khi nhận được tin nhắn WebSocket).
- **`void *parameter`**: Con trỏ vạn năng trong C/FreeRTOS, cho phép truyền bất kỳ dữ liệu cấu hình nào vào Task khi khởi tạo.

---

## 12. Hướng Dẫn Đọc Code Theo Thứ Tự Cho Người Mới

Nếu bạn mới bắt đầu học C/C++ Embedded, hãy đọc các file theo trình tự logic sau:

1. [config.h](file:///c:/Users/LENOVO/Desktop/DATN_START/firmware/include/config.h): Nắm toàn bộ sơ đồ chân GPIO và chu kỳ thời gian.
2. [system_types.h](file:///c:/Users/LENOVO/Desktop/DATN_START/firmware/include/system_types.h): Hiểu các kiểu struct và enum truyền nhận dữ liệu.
3. [actuator.h](file:///c:/Users/LENOVO/Desktop/DATN_START/firmware/include/actuator.h) & [actuator.cpp](file:///c:/Users/LENOVO/Desktop/DATN_START/firmware/src/actuator.cpp): Xem cách ánh xạ từ ID sang chân phần cứng và ghi băm xung PWM.
4. [rtos_app.h](file:///c:/Users/LENOVO/Desktop/DATN_START/firmware/include/rtos_app.h) & [rtos_app.cpp](file:///c:/Users/LENOVO/Desktop/DATN_START/firmware/src/rtos_app.cpp): Học cách FreeRTOS truyền nhận tin nhắn giữa các Task qua Queue và Mutex.
5. [main.cpp](file:///c:/Users/LENOVO/Desktop/DATN_START/firmware/src/main.cpp): Xem cách 3 Task chính được tạo và gán vào Core 0 / Core 1.
6. [failsafe.cpp](file:///c:/Users/LENOVO/Desktop/DATN_START/firmware/src/failsafe.cpp): Xem cách tiếp nhận lệnh từ Web và xử lý ngắt an toàn.
7. [sensors.cpp](file:///c:/Users/LENOVO/Desktop/DATN_START/firmware/src/sensors.cpp): Xem quy trình đọc, xử lý lọc nhiễu và quy đổi đơn vị cảm biến.
8. Các file mạng (`wifi_manager.cpp`, `web_api.cpp`, `ota_update.cpp`, `network_task.cpp`): Tìm hiểu luồng giao tiếp HTTP, WebSocket và Web OTA.

---

## 13. Quy Trình Kiểm Thử Thực Tế

1. **Kiểm tra phần cứng an toàn:**
   - Cấp nguồn 5V/12V trước, chưa nối tải 220V.
   - Quan sát LED hệ thống: Lúc boot tất cả 10 ngõ ra phải ở trạng thái OFF.
2. **Kiểm tra kết nối Web Dashboard:**
   - Kết nối Wi-Fi AP `DATN_AIOT_LETTUCE` (IP `192.168.4.1` hoặc `http://datn.local`).
   - Kiểm tra các thông số cảm biến hiển thị trên màn hình.
3. **Kiểm tra điều khiển MANUAL:**
   - Bật/tắt từng Relay và kéo thanh trượt PWM cho từng Đèn/Bơm/Quạt.
   - Xác nhận thiết bị phản hồi mượt mà không bị giật lùi công tắc.
4. **Kiểm tra Fail-safe:**
   - Đang bật bơm/đèn, tiến hành tắt Wi-Fi máy tính hoặc đóng tab trình duyệt ➔ Sau 10 giây toàn bộ tải phải tự động ngắt (OFF).
