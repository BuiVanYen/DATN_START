# CẨM NANG KỊCH BẢN LẬP TRÌNH VÀ VẬN HÀNH HỆ THỐNG THỦY CANH NFT

> Phiên bản kịch bản: 2.1  
> Ngày cập nhật: 22/07/2026  
> Bộ điều khiển: ESP32-S3-WROOM-1-N16R8, 16 MB Flash, 8 MB PSRAM  
> Chế độ tuần hoàn được chọn: 15 phút BẬT / 15 phút TẮT  
> Đối tượng trồng: xà lách mỡ, bể làm việc khoảng 40 L

Tài liệu này là đặc tả đầu vào để lập trình firmware. Nội dung mô tả hệ thống phải hoạt động như thế nào sau khi hoàn thành, không phải báo cáo hiện trạng code.

---

## 1. Mục tiêu và nguyên tắc bắt buộc

### 1.1. Mục tiêu

- Điều khiển hệ thống NFT theo chu kỳ tuần hoàn 15 phút BẬT / 15 phút TẮT.
- Giữ lại lớp nước sâu khoảng 1-2 cm trong máng khi bơm tắt nhờ gờ cơ khí ở đầu và cuối máng.
- Đo ánh sáng, nhiệt độ, độ ẩm, nhiệt độ nước, pH, TDS/EC, lưu lượng và mức nước.
- Điều khiển đèn, quạt, tuần hoàn, sục khí, cấp nước và ba bơm định lượng.
- Cho phép vận hành MANUAL, AUTO, CALIBRATION và COMMISSIONING.
- Châm dinh dưỡng theo kết quả đo, không châm theo thời gian cố định mù.
- Mọi lỗi nguy hiểm phải đưa bơm hóa chất về OFF và báo rõ nguyên nhân.

### 1.2. Các bất biến an toàn

1. Chỉ `TaskSafetyControl` được phép thay đổi đầu ra vật lý.
2. Bơm A, bơm B và bơm pH Down không bao giờ chạy đồng thời.
3. Bơm refill không chạy đồng thời với bất kỳ bơm định lượng nào.
4. Dữ liệu `STALE`, `ERROR`, ngoài dải hoặc chưa hiệu chuẩn không được dùng để châm.
5. Mỗi bơm phải có timeout cứng, quota theo lần, theo giờ và theo ngày.
6. Boot, reset, brownout, watchdog và OTA luôn bắt đầu với tải nguy hiểm ở OFF.
7. Mất điện giữa liều không được tiếp tục liều cũ sau khi khởi động lại.
8. Pha TẮT không được kéo dài quá deadline chỉ vì đo hoặc châm chưa xong.
9. Lệnh MANUAL vẫn phải tuân thủ interlock, timeout, quota và E-stop.
10. Kịch bản 15/15 chỉ mở AUTO sau khi máng đã qua kiểm thử giữ nước và lứa cây đã sẵn sàng.

---

## 2. Phần cứng và các kênh vào/ra

### 2.1. Bộ điều khiển

- Module được chọn: `ESP32-S3-WROOM-1-N16R8`.
- Dung lượng: 16 MB Flash và 8 MB PSRAM.
- Phân vùng Flash phải có vùng ứng dụng chính, vùng OTA dự phòng và vùng NVS phù hợp 16 MB.
- Không dùng dung lượng PSRAM để chứa trạng thái an toàn bắt buộc; trạng thái điều khiển cốt lõi phải hoạt động được khi PSRAM lỗi hoặc không khởi tạo.
- Sau khi thay module, phải kiểm tra lại nguồn, chân strapping, PSRAM mode, Flash mode, USB và khả năng OTA trên bo thật.

### 2.2. Mười thiết bị đầu vào, mười một kênh đo và Sơ đồ GPIO

| Thiết bị | Kênh dữ liệu | GPIO ESP32-S3 | Chuẩn giao tiếp / Mức tín hiệu | Tên Macro Firmware | Vai trò chính |
|---|---|---|---|---|---|
| BH1750 | Lux | GPIO5 (SDA), GPIO4 (SCL) | I2C (Địa chỉ 0x23 / 0x5C) | `PIN_SDA`, `PIN_SCL` | Giám sát ánh sáng tương đối |
| SHT30/SHT31 | Nhiệt độ KK, Độ ẩm | GPIO5 (SDA), GPIO4 (SCL) | I2C (Địa chỉ 0x44) | `PIN_SDA`, `PIN_SCL` | Điều khiển quạt & cảnh báo môi trường |
| DS18B20 | Nhiệt độ nước | GPIO14 | OneWire (Pull-up 4.7k) | `PIN_DS18B20` | Bù nhiệt EC/TDS & bảo vệ pha TẮT |
| Đầu dò pH | pH | GPIO2 | ADC1_CH1 (Analog 0-3.3V) | `PIN_ADC_PH` | Quyết định châm pH Down |
| Đầu dò TDS/EC | TDS/EC25 | GPIO1 (ADC), GPIO42 (EN) | ADC1_CH0 + Gate P-MOSFET EN | `PIN_ADC_TDS`, `PIN_EN_TDS` | Quyết định châm A/B; EN=LOW bật nguồn |
| Flow sensor | Lưu lượng tổng | GPIO41 | Interrupt (FALLING, Pull-up) | `PIN_FLOW` | Xác nhận bơm tuần hoàn tạo dòng |
| Phao bồn chính | Mực nước bồn | GPIO40 | Digital Input (LOW=OK, HIGH=LOW) | `PIN_LEVEL1` | Chống chạy khô & yêu cầu refill |
| Phao chai A | Trạng thái chai A | GPIO39 | Digital Input (1=CÒN, 0=CẠN) | `PIN_LEVEL2` | Cho phép châm A |
| Phao chai B | Trạng thái chai B | GPIO38 | Digital Input (1=CÒN, 0=CẠN) | `PIN_LEVEL3` | Cho phép châm B |
| Phao pH Down | Trạng thái chai pH | GPIO21 | Digital Input (1=CÒN, 0=CẠN) | `PIN_LEVEL4` | Cho phép châm pH Down |

*Ghi chú*: SHT31 là 1 thiết bị vật lý nhưng trả về 2 kênh dữ liệu độc lập (Nhiệt độ & Độ ẩm), nên giao diện hiển thị đúng 11 giá trị đo.

### 2.3. Mười tải chính và Sơ đồ điều khiển GPIO

| Nhóm | Tải | GPIO ESP32-S3 | Loại điều khiển / Mức kích | Tên Macro Firmware |
|---|---|---|---|---|
| Chiếu sáng | Đèn tầng 1 | GPIO17 | PWM MOSFET (Duty 0-100%) | `DEN1` |
| Chiếu sáng | Đèn tầng 2 | GPIO18 | PWM MOSFET (Duty 0-100%) | `DEN2` |
| Thông gió | Quạt tầng 1 | GPIO11 | PWM MOSFET (Duty 0-100%) | `QUAT1` |
| Thông gió | Quạt tầng 2 | GPIO10 | PWM MOSFET (Duty 0-100%) | `QUAT2` |
| Định lượng | Bơm A (Nutrient A) | GPIO13 | PWM MOSFET (Duty 0-100%) | `BOMLL1` |
| Định lượng | Bơm B (Nutrient B) | GPIO12 | PWM MOSFET (Duty 0-100%) | `BOMLL2` |
| Định lượng | Bơm pH Down | GPIO8 | PWM MOSFET (Duty 0-100%) | `BOMLL3` |
| Cấp nước | Bơm sạch 12 V (Refill) | GPIO9 | PWM MOSFET (Duty 0-100%) | `BOM12V` |
| Tuần hoàn | Bơm chìm 220 V NFT | GPIO7 | Relay 2 (Opto PC817 + Transistor, HIGH=ON) | `IN_RL1` |
| Oxy/trộn | Máy sục khí 220 V | GPIO6 | Relay 1 (Opto PC817 + Transistor, HIGH=ON) | `IN_RL2` |

**Đầu ra phụ hỗ trợ**:
- **Buzzer**: GPIO47 (`BUZZER`, Digital Output, HIGH = Kêu).
- **LED hệ thống**: GPIO48 (`LED_SYS`, Digital Output, HIGH = Sáng).
- **Công tắc nguồn TDS**: GPIO42 (`PIN_EN_TDS`, Output P-MOSFET, LOW = Cấp điện cho mạch TDS).

### 2.4. Điều kiện cơ khí cho chế độ 15/15

- Máng có gờ giữ nước cao khoảng 1-2 cm ở đầu và cuối máng theo thiết kế hiện tại.
- Khi bơm tắt, phải đo độ sâu thực tế tại đầu, giữa và cuối từng máng.
- Không được có siphon kéo cạn máng, tràn ngược, điểm khô cục bộ hoặc máng võng tạo vùng ngập quá sâu.
- Rễ và giá thể phải tiếp xúc được với nước giữ lại trong toàn bộ 15 phút TẮT.
- Mỗi lứa cây mới phải thử một pha TẮT có giám sát trước khi mở AUTO 15/15.
- Relay/contactor và bơm phải chịu được khoảng 48 lần khởi động mỗi ngày.

---

## 3. Kiến trúc phần mềm đích

### 3.1. Ba task chính và Phân chia Nhân CPU (Dual-Core Assignment)

ESP32-S3 sở hữu 2 nhân vi xử lý độc lập (`Core 0` và `Core 1`). Để đảm bảo tính thời gian thực và an toàn cao nhất, 3 Task chính được phân chia nhân và độ ưu tiên FreeRTOS như sau:

| Tên Task | CPU Core | Độ ưu tiên (Priority) | Dung lượng Stack | Vai trò chính |
|---|---|---|---|---|
| `TaskSafetyControl` | **Core 1** (APP CPU) | **Mức 5** (Cao nhất) | 8192 Bytes | Máy trạng thái FSM, kiểm soát duy nhất đầu ra vật lý, interlock, timeout & fault |
| `TaskSensors` | **Core 1** (APP CPU) | **Mức 3** (Trung bình) | 4096 Bytes | Đọc cảm biến định kỳ (I2C, OneWire, ADC, Flow), lọc nhiễu, gắn nhãn quality |
| `TaskNetwork` | **Core 0** (PRO CPU) | **Mức 2** (Thấp hơn) | 8192 Bytes | Quản lý Wi-Fi stack, AsyncWebServer, WebSocket, MQTT client & OTA update |

*Nguyên tắc phân nhân*:
- **Core 0 (Network Core)**: Chuyên trách Wi-Fi/TCP-IP stack và truyền nhận dữ liệu mạng. Khi Wi-Fi mất kết nối hoặc khi tải file OTA nặng, Core 0 có thể bị chiếm dụng nhưng **tuyệt đối không gây ảnh hưởng đến Core 1**.
- **Core 1 (Control Core)**: Dành riêng cho logic ứng dụng điều khiển thời gian thực và an toàn (`TaskSafetyControl` + `TaskSensors`). Nhờ đó, máy trạng thái 15/15 và các bảo vệ ngắt bơm khẩn cấp hoạt động liên tục, chính xác từng millisecond mà không bị nghẽn do mạng.

#### Chi tiết nhiệm vụ từng Task:

#### TaskSensors (Core 1)

- Đọc cảm biến theo lịch riêng cho từng nhóm.
- Lọc nhiễu, kiểm tra phạm vi và gắn chất lượng mẫu.
- Không tự bật hoặc tắt tải.
- Gửi `SensorSample` vào `SensorQueue`.

#### TaskSafetyControl (Core 1)

- Là chủ sở hữu duy nhất của đầu ra.
- Chạy máy trạng thái AUTO, MANUAL, CALIBRATION và COMMISSIONING.
- Xử lý chu kỳ 15/15, refill, flow, Quiet Window, dosing, timeout và fault.
- Nhận lệnh nghiệp vụ từ `CommandQueue`; không nhận lệnh GPIO thô.

#### TaskNetwork (Core 0)

- Quản lý Wi-Fi, Web, WebSocket, MQTT và OTA.
- Kiểm tra cú pháp, quyền truy cập, ID lệnh và TTL.
- Chỉ gửi lệnh nghiệp vụ vào queue.
- Mất mạng không được làm dừng AUTO cục bộ nếu hệ thống vẫn an toàn.

### 3.2. Dữ liệu cảm biến chuẩn

Mỗi mẫu cần tối thiểu các trường:

```text
SensorSample {
  sensor_id
  value
  unit
  quality: GOOD | STALE | ERROR | UNSTABLE | UNCALIBRATED
  sampled_at_ms
  age_ms
  raw_value
  calibration_id
}
```

### 3.3. Lịch đọc cảm biến

| Nhóm | Chu kỳ đề xuất | Ghi chú |
|---|---:|---|
| Phao mức | 100 ms | Debounce liên tục 2 giây |
| Flow | Cửa sổ 1 giây | Chỉ đánh giá lỗi khi bơm được lệnh ON |
| SHT31 | 2-5 giây | Có giới hạn tần suất và kiểm tra mất kết nối |
| BH1750 | 2-5 giây | Lux dùng giám sát tương đối |
| DS18B20 | 5-30 giây | Đọc không blocking |
| pH và EC/TDS | Theo Quiet Window | Không đọc điều khiển khi tải gây nhiễu đang chạy |

Không gom tất cả cảm biến vào một chu kỳ 30 giây duy nhất vì phao và flow cần phản ứng nhanh hơn nhiều.

### 3.4. Watchdog và heartbeat

- Mỗi task cập nhật heartbeat độc lập.
- Watchdog chỉ được feed khi các heartbeat bắt buộc còn hợp lệ.
- Khi watchdog reset, phần cứng và firmware phải bảo đảm tải nguy hiểm OFF ngay từ lúc boot.
- Ghi lại `reset_reason`, `boot_id`, trạng thái trước reset và fault còn treo.

---

## 4. Cấu hình bắt buộc trước khi mở AUTO

| Nhóm | Cấu hình cần có |
|---|---|
| Bể | Thể tích làm việc thực, không mặc định tuyệt đối là 40 L |
| Chu kỳ | `circ_on_s=900`, `circ_off_s=900`, guard và timeout mồi bơm |
| Cơ khí | Cờ đã nghiệm thu giữ nước, mã lứa cây và xác nhận rễ sẵn sàng |
| Bơm A/B/pH | Số giây chạy thử bằng nước; mL/s, duty, timeout và ngày hiệu chuẩn sau khi đã đo |
| Dinh dưỡng | Tên sản phẩm, tỷ lệ A:B, hướng dẫn của nhà sản xuất |
| EC | EC25 mục tiêu, deadband, giới hạn vòng 1/vòng 2 |
| TDS | Hệ số quy đổi hiển thị; không dùng TDS thay EC25 trong lõi điều khiển |
| pH | Dải mục tiêu, deadband và lượng micro-dose tối đa |
| Flow | Lưu lượng khỏe, thời gian mồi, số cửa sổ lỗi và số lần retry |
| Refill | Thời gian bơm xấu nhất, `fill_max_time` và bảo vệ tràn độc lập |
| Quota | mL/lần, mL/giờ và mL/ngày cho từng bơm |
| Timeline châm | Các thời gian châm, trộn, chờ tĩnh, đo và dự phòng nhập theo giây trên Web |
| Thời gian | Múi giờ, NTP và/hoặc RTC nếu lịch phải chạy độc lập |

Toàn bộ cấu hình phải có version, CRC và giới hạn hợp lệ. Cấu hình lỗi đưa hệ thống về `SAFE_BOOT`, không tự thay bằng giá trị ngẫu nhiên.

### 4.1. Thông số thực nghiệm bơm & dung dịch đã hiệu chuẩn

#### a) Tốc độ lưu lượng bơm định lượng 12V thực tế
- **Kết quả đo thực tế:** 200 mL trong 130 giây.
- **Tốc độ bơm thực tế (Flow Rate):** $1.538 \text{ mL/giây}$ (tương đương $0.65 \text{ giây/mL}$).

#### b) Thông số dung dịch dinh dưỡng Hydro Umat V (Group A & B)
- **Tỷ lệ pha chuẩn nhà sản xuất:** 50 mL dung dịch A + 50 mL dung dịch B cho 20 Lít nước sạch (tương đương $2.5 \text{ mL/Lít}$).
- **Áp dụng cho bể 40 Lít (bể làm việc):**
  - **Pha bể mới từ 0 PPM lên 600 - 700 PPM:** Cần 100 mL dung dịch A và 100 mL dung dịch B.
    - *Thời gian bơm A:* $100 / 1.538 = \mathbf{65.0 \text{ giây}}$ (1 phút 5 giây).
    - *Thời gian bơm B:* $100 / 1.538 = \mathbf{65.0 \text{ giây}}$ (1 phút 5 giây).
  - **Vi liều châm bù định kỳ khi sụt 100 PPM:** Cần 20 mL dung dịch A và 20 mL dung dịch B.
    - *Thời gian bơm A:* $20 / 1.538 = \mathbf{13.0 \text{ giây}}$.
    - *Thời gian bơm B:* $20 / 1.538 = \mathbf{13.0 \text{ giây}}$.
- **Ngưỡng PPM khuyến nghị theo giai đoạn rau ăn lá (Xà lách):**
  - Cây con (3 - 12 ngày tuổi): 300 - 400 PPM (~0.6 - 0.8 EC)
  - Tăng trưởng (12 - 20 ngày tuổi): 600 - 750 PPM (~1.2 - 1.5 EC)
  - Trưởng thành (20 - 37 ngày tuổi): 800 - 1000 PPM (~1.6 - 2.0 EC)
  - Thu hoạch (37+ ngày): 700 - 800 PPM (~1.4 - 1.6 EC)

#### c) Thông số dung dịch hạ pH (General Hydroponics pH Down)
- **Liều lượng vi liều an toàn cho bể 40 Lít:** Châm từng micro-dose từ 2 mL đến 5 mL mỗi lần.
- **Thời gian chạy bơm pH Down:**
  - *Vi liều 2 mL (hạ ~0.2 pH):* $2 / 1.538 = \mathbf{1.3 \text{ giây}}$.
  - *Vi liều 5 mL (hạ ~0.4 pH):* $5 / 1.538 = \mathbf{3.25 \text{ giây}}$.
  - *Vi liều 10 mL (hạ ~0.8 pH):* $10 / 1.538 = \mathbf{6.5 \text{ giây}}$.

---

## 5. Máy trạng thái tổng

Thứ tự ưu tiên:

```text
EMERGENCY/FAULT > SAFE_BOOT/OTA > CALIBRATION/MANUAL > AUTO
```

| Trạng thái | Mục đích | Tải được phép |
|---|---|---|
| `SAFE_BOOT` | Tự kiểm tra và xác minh cấu hình | Tải chính OFF |
| `COMMISSIONING` | Chạy thử bằng nước sạch | Tải được cấp quyền; khóa hóa chất |
| `CIRC_ON` | Tuần hoàn 15 phút | Tuần hoàn + sục khí, đèn/quạt theo lịch |
| `CIRC_OFF` | Nghỉ 15 phút | Tuần hoàn OFF; sục theo quy trình đo/trộn |
| `QUIET_MEASURE` | Đo pH/EC ít nhiễu | Tải gây nhiễu OFF |
| `DOSE_AB` | Châm dinh dưỡng | Một bơm A hoặc B tại một thời điểm |
| `DOSE_PH` | Châm pH Down | Chỉ bơm pH Down |
| `MIX` | Trộn sau châm/refill | Sục khí ON; tuần hoàn theo quy trình |
| `REFILL` | Cấp nước sạch | Refill ON; dosing OFF |
| `MANUAL` | Bảo trì có TTL | Chỉ tải được cấp quyền |
| `CALIBRATION` | Hiệu chuẩn từng phần tử | Một mục tiêu/lần, hold-to-run |
| `OTA` | Cập nhật firmware | Tải nguy hiểm OFF |
| `FAULT` | Cô lập sự cố | Theo ma trận lỗi; dosing OFF |

Mọi thao tác cưỡng bức như boot, refill, flow retry, dosing, OTA hoặc fault đều ghi đè timer 15/15. Khi hoàn tất, hệ thống bắt đầu một pha `CIRC_ON` mới; không nối tiếp phần thời gian còn lại của pha cũ.

---

## 6. Kịch bản chu kỳ 15 phút BẬT / 15 phút TẮT

### 6.1. Pha `CIRC_ON`

1. Bật bơm tuần hoàn và máy sục khí.
2. Chờ thời gian mồi, ví dụ 10 giây.
3. Xác nhận flow đạt ngưỡng trong các cửa sổ liên tiếp.
4. Chỉ bắt đầu đếm 900 giây sau khi flow đã hợp lệ.
5. Trong pha ON:
   - phao và flow được theo dõi liên tục;
   - DS18B20, SHT31 và BH1750 đọc theo lịch;
   - pH/TDS không được dùng để ra quyết định dosing;
   - đèn/quạt chạy theo lịch và thuật toán môi trường.
6. Khi đủ 900 giây, nếu không có quy trình ưu tiên, chuyển sang `CIRC_OFF`.

### 6.2. Pha `CIRC_OFF`

1. Tắt bơm tuần hoàn.
2. Flow giảm về 0 là trạng thái bình thường; không báo `FAULT_FLOW` khi bơm đang OFF.
3. Nước được giữ lại 1-2 cm trong máng nhờ gờ cơ khí.
4. Pha OFF tối đa 900 giây cộng guard nhỏ để chuyển trạng thái.
5. Nếu nhiệt độ nước cao, rễ có dấu hiệu thiếu nước hoặc pha OFF vượt deadline, kết thúc OFF sớm và vào `CIRC_ON`.
6. Nếu có yêu cầu đo/châm, chạy timeline tại Chương 7.
7. Nếu không cần đo/châm, sục khí có thể chạy theo cấu hình nhưng không được làm sai điều kiện Quiet Window kế tiếp.

### 6.3. Điều kiện kết thúc pha OFF sớm

- Nhiệt độ nước đạt hoặc vượt ngưỡng bảo vệ, đề xuất 25 °C.
- DS18B20 `STALE/ERROR` và không thể xác nhận nhiệt độ an toàn.
- Mức bồn không hợp lệ hoặc báo LOW.
- Người vận hành yêu cầu dừng khẩn.
- Watchdog, task heartbeat hoặc queue điều khiển có lỗi.
- Deadline `CIRC_OFF` bị vượt.

### 6.4. Quy tắc flow

- Chỉ đánh giá flow sau lệnh bật tuần hoàn và sau thời gian mồi.
- Flow thấp liên tục ba cửa sổ: tắt bơm, chờ backoff rồi retry.
- Retry tối đa hai lần.
- Vẫn thấp: khóa dosing, giữ sục khí nếu an toàn và chốt `FAULT_FLOW`.
- Flow khác 0 kéo dài khi bơm OFF: cảnh báo `FLOW_WHILE_OFF` để kiểm tra siphon hoặc cảm biến kẹt.

---

## 7. Quiet Window và hai vòng châm trong 15 phút nghỉ

### 7.1. Điều kiện bắt đầu

Chỉ đo/châm khi tất cả điều kiện sau đúng:

- Mức bồn hợp lệ.
- Không có refill, fault, OTA hoặc lệnh MANUAL xung đột.
- Cảm biến cần thiết đã hiệu chuẩn và còn hạn.
- Chai cần dùng chưa cạn.
- Quota còn đủ.
- Thời gian còn lại của pha OFF đủ cho toàn bộ bước và safety margin.

### 7.2. Timeline cấu hình trên Web cho tối đa hai vòng

Kịch bản không chốt cứng 5 phút, 300 giây hay 150 giây cho bước châm/trộn khi chưa đo lưu lượng và chưa thử độ đồng đều của bể. Người vận hành nhập các thời gian sau trên Web:

| Thứ tự | Công việc | Tham số Web, đơn vị giây |
|---:|---|---|
| 1 | Tắt tuần hoàn và sục, chờ nước tĩnh trước M0 | `quiet_settle_before_m0_s` |
| 2 | Đo nền M0: nhiệt độ nước, pH, EC/TDS | `measure_m0_max_s` |
| 3 | Quyết định và châm vòng 1 | `dose_round1_max_s` |
| 4 | Sục khí trộn vòng 1 | `mix_round1_s` |
| 5 | Tắt sục, chờ nước tĩnh trước M1 | `settle_round1_s` |
| 6 | Đo xác nhận M1 | `measure_m1_max_s` |
| 7 | Micro-dose vòng 2 nếu được phép | `dose_round2_max_s` |
| 8 | Sục khí trộn vòng 2 | `mix_round2_s` |
| 9 | Tắt sục, chờ nước tĩnh trước M2 | `settle_round2_s` |
| 10 | Đo xác nhận cuối M2 | `measure_m2_max_s` |
| 11 | Ghi log, dự phòng và chuẩn bị bật tuần hoàn | `safety_margin_s` |

`dose_round1_max_s` và `dose_round2_max_s` là cửa sổ tối đa của cả vòng châm. Với dinh dưỡng A/B, cửa sổ này phải bao gồm thời gian chạy A, `ab_separation_mix_s` và thời gian chạy B; với pH Down, nó chỉ bao gồm bơm pH và các bước bảo vệ liên quan.

Web phải hiển thị tổng số giây của timeline và số giây còn lại trong pha OFF:

`timeline_total_s = quiet_settle_before_m0_s + measure_m0_max_s + dose_round1_max_s + mix_round1_s + settle_round1_s + measure_m1_max_s + dose_round2_max_s + mix_round2_s + settle_round2_s + measure_m2_max_s + safety_margin_s`

`off_remaining_s = circ_off_s - timeline_total_s`

- Không tự điền 300 giây hoặc 150 giây làm kết luận kỹ thuật.
- Các ô cấu hình phải có đơn vị giây, giới hạn hợp lệ, nút lưu và giá trị đang có hiệu lực.
- Nếu timeline hai vòng không vừa `circ_off_s`, Web phải cảnh báo; lõi điều khiển không được bắt đầu vòng 2.
- Nếu ngay cả vòng 1 không đủ thời gian cùng `safety_margin_s`, khóa châm trong pha OFF đó.
- Sau mỗi lần thay đổi thời gian, phải chạy thử bằng nước và ghi lại kết quả trước khi cấp quyền dùng hóa chất.

Chưa đo mL/s thì các thời gian bơm chỉ dùng cho `COMMISSIONING` bằng nước sạch. Sau khi hiệu chuẩn, thời gian chạy thực tế của từng bơm được tính từ `mL yêu cầu / mL mỗi giây`; `dose_round1_max_s` và `dose_round2_max_s` chỉ là trần timeout, không thay cho hiệu chuẩn lưu lượng.

### 7.3. Cách đo pH và EC/TDS

1. Bảo đảm đèn, quạt, tuần hoàn và sục khí ở trạng thái không gây nhiễu theo thiết kế đo.
2. DS18B20 phải trả về nhiệt độ hợp lệ ($T_{\text{water}}$).
3. **Thu thập mẫu pH**: pH lấy hai cụm độc lập, mỗi cụm 30 mẫu ADC (`PIN_ADC_PH`).
4. Mỗi cụm bỏ 20% mẫu thấp nhất và 20% cao nhất, lấy trung bình 60% mẫu giữa để tính $V_{\text{pH\_avg}}$.
5. **Công thức nội suy pH 2 điểm (pH 4.00 & pH 7.00)**:
   $$pH = pH_1 + \frac{(V_{\text{pH}} - V_1) \cdot (pH_2 - pH_1)}{V_2 - V_1}$$
   - Hai cụm pH lệch quá 0,15 pH thì mẫu chốt là `UNSTABLE`.
6. **Thu thập mẫu EC/TDS**: Bật nguồn TDS (`PIN_EN_TDS = LOW`), chờ 100-200 ms cho mạch ổn định, lấy hai cụm 30 mẫu ADC (`PIN_ADC_TDS`) và tắt nguồn ngay sau đo (`PIN_EN_TDS = HIGH`).
7. **Công thức bù nhiệt EC về 25 °C ($EC_{25}$)**:
   $$EC_{25} = \frac{EC_T}{1 + \alpha \cdot (T_{\text{water}} - 25.0)}$$
   *(Trong đó hệ số bù nhiệt chuẩn $\alpha = 0.019 / ^\circ\text{C}$)*
   - Hai cụm EC/TDS lệch quá 5% thì mẫu chốt là `UNSTABLE`.
   - TDS (ppm) chỉ là giá trị quy đổi hiển thị: $TDS = EC_{25} \cdot 500$ (hoặc hệ số quy đổi cấu hình trên Web).
8. **Công thức tính lưu lượng nước (Flow sensor)**:
   $$Q (\text{Lít/phút}) = \frac{f_{\text{pulses}}}{K_{\text{factor}}}$$
   *(Với $f_{\text{pulses}}$ là số xung ngắt thu thập trong cửa sổ 1 giây)*
9. ADC bão hòa (dưới 50 hoặc trên 4000), giá trị ngoài dải hoặc calibration hết hạn đều khóa dosing.

### 7.4. Quyết định châm

Mỗi pha OFF chỉ xử lý một nhóm hiệu chỉnh:

1. Nếu EC thấp: châm A/B; chưa châm pH Down trong cùng pha.
2. Nếu EC đạt và pH cao: có thể châm pH Down.
3. Nếu cả EC thấp và pH cao: ưu tiên A/B; đo lại pH ở pha OFF sau.
4. Nếu EC cao: hệ thống không có cơ cấu tự giảm; báo người vận hành pha loãng/thay một phần nước.
5. Nếu pH thấp: hệ thống không có bơm pH Up; khóa pH Down và báo người vận hành.

### 7.5. Châm A/B

1. Tính mL cần châm từ EC25, thể tích thực, deadband, giới hạn tăng tối đa và công thức của sản phẩm.
2. Giới hạn kết quả theo quota.
3. Ghi `dose_in_progress` trước khi bật bơm.
4. Bật sục khí để tạo chuyển động trong bể.
5. Chạy bơm A theo mL/s đã hiệu chuẩn rồi tắt.
6. Sục trộn giữa A và B theo `ab_separation_mix_s` đã nhập trên Web và đã thử bằng nước.
7. Chạy bơm B theo tỷ lệ sản phẩm rồi tắt.
8. Chạy pha MIX theo `mix_round1_s` hoặc `mix_round2_s` đang có hiệu lực.
9. Đo lại trước khi quyết định vòng 2.
10. Chỉ xóa cờ giao dịch sau khi quy trình kết thúc hợp lệ.

### 7.6. Châm pH Down

1. Chỉ châm khi pH cao hơn ngưỡng trên và EC đã ổn định.
2. Dùng micro-dose nhỏ, không cố đưa pH về đúng mục tiêu trong một lần.
3. Ghi cờ giao dịch, chạy bơm theo mL/s đã hiệu chuẩn rồi tắt.
4. Trộn, chờ tĩnh và đo lại.
5. Tối đa hai micro-dose trong một pha OFF nếu đủ thời gian và quota.
6. Nếu pH vẫn cao, hoãn lần tiếp theo; không châm liên tục để đuổi theo số đo.

### 7.7. Quy tắc deadline

- Mỗi bước có deadline riêng, được tính từ tham số số giây đang có hiệu lực trên Web.
- Khi thời gian còn lại không đủ cho `dose + mix + settle + measure + safety_margin_s`, không bắt đầu vòng mới.
- Mọi bơm định lượng phải kết thúc trước `circ_off_s - safety_margin_s`; không dùng mốc 14:00 cố định nếu người vận hành đã cấu hình biên dự phòng hợp lệ khác.
- Đến 15:00 phải chuyển sang bật tuần hoàn, trừ khi một fault yêu cầu giữ OFF.
- Nếu vượt deadline, tạo `CIRC_OFF_OVERRUN`, khóa vòng tiếp theo và yêu cầu kiểm tra.

---

## 8. Các kịch bản phụ bắt buộc

### 8.1. Khởi động an toàn

1. Phần cứng giữ gate/relay OFF trong thời gian reset.
2. Firmware ghi OFF cho mọi tải trước khi đọc NVS.
3. Kiểm tra nguyên nhân reset, cấu hình, calibration và cờ giao dịch dở dang.
4. Nếu `dose_in_progress=true`, tạo `DOSE_INTERRUPTED`; không chạy tiếp liều cũ.
5. Kiểm tra phao, cảm biến, thời gian và flow path.
6. Nếu đủ điều kiện AUTO 15/15, bắt đầu bằng một pha `CIRC_ON` mới.

### 8.2. Refill

1. Phao bồn phải báo LOW ổn định 2 giây.
2. Tắt tuần hoàn và khóa dosing.
3. Bật refill ở duty đã hiệu chuẩn.
4. Dừng khi phao về OK ổn định, phao cao độc lập tác động hoặc hết `fill_max_time`.
5. Timeout/tràn: tắt refill, chốt fault và chờ xác nhận thủ công.
6. Refill thành công: tuần hoàn + sục khí để trộn, xác nhận flow rồi đo hóa học ở pha phù hợp.

Một phao bồn không đủ vừa làm điểm refill vừa bảo vệ tràn. AUTO không giám sát cần phao mức cao/ngắt cứng độc lập hoặc cảm biến rò.

### 8.3. MANUAL

- Mỗi lệnh có `command_id`, TTL, người gửi và lý do.
- Bơm định lượng dùng hold-to-run hoặc lệnh có thời lượng/mL rõ ràng.
- Hết TTL phải tự tắt.
- Không cho phép gửi số GPIO trực tiếp từ Web/MQTT.
- MANUAL không được bỏ qua interlock, quota và timeout.

### 8.4. CALIBRATION

- Mỗi lần chỉ hiệu chuẩn một cảm biến hoặc một bơm.
- Bơm hóa chất được thay bằng nước sạch khi hiệu chuẩn lưu lượng.
- Lưu mL/s, sai số, duty, ngày và người thực hiện.
- pH cần dung dịch chuẩn pH 4,00 và 7,00 khi hiệu chuẩn chính xác.
- EC/TDS cần dung dịch chuẩn phù hợp đầu dò khi hiệu chuẩn chính xác.

### 8.5. OTA

- Chỉ bắt đầu khi không refill/dosing và không có giao dịch dở dang.
- Đưa tải nguy hiểm OFF.
- Xác minh file, kích thước và chữ ký/checksum theo thiết kế.
- Nếu OTA thất bại, quay lại image trước và vẫn boot với tải OFF.

---

## 9. Điều khiển đèn, quạt và môi trường

### 9.1. Đèn

- Lịch đèn là điều khiển chính.
- BH1750 dùng giám sát và hiệu chỉnh tương đối tại vị trí đã cố định.
- Lux không thay thế PPFD/DLI trong đánh giá ánh sáng cho cây.
- PWM thay đổi theo ramp để tránh nhấp nháy và dòng khởi động lớn.
- Có `max_led_duty`, lịch ON/OFF và fallback khi BH1750 lỗi.

### 9.2. Quạt

Ví dụ luật điều khiển sau khi hiệu chỉnh:

| Sai lệch nhiệt độ | Duty quạt |
|---|---:|
| Nhỏ hơn hoặc bằng mục tiêu | 30% khi cần thông gió nền |
| Cao hơn 0-2 °C | 40% |
| Cao hơn 2-4 °C | 70% |
| Cao hơn 4 °C | 100% |

- Hysteresis đề xuất: 1 °C.
- Thời gian giữ mức tối thiểu: 60 giây.
- Khi SHT31 lỗi: chạy duty dự phòng đã cấu hình và báo lỗi.

### 9.3. Nhiệt độ nước

- Dải mục tiêu phải cấu hình theo mùa và giống cây.
- Khi nhiệt độ nước từ 25 °C trở lên, kết thúc pha OFF sớm và ưu tiên tuần hoàn/sục khí nếu mức/flow an toàn.
- Khi DS18B20 lỗi, khóa dosing và không tiếp tục OFF kéo dài.

---

## 10. Ma trận lỗi và phản ứng

| Lỗi | Phản ứng bắt buộc |
|---|---|
| `FAULT_LEVEL_MAIN` | Tắt tuần hoàn, refill và dosing; kiểm tra phao/dây |
| `REFILL_TIMEOUT` | Tắt refill; khóa dosing; chờ xác nhận |
| `OVERFLOW_HIGH` | Tắt refill ngay; báo khẩn |
| `FAULT_FLOW` | Tắt tuần hoàn sau retry; dosing OFF; giữ sục nếu an toàn |
| `FLOW_WHILE_OFF` | Cảnh báo siphon/cảm biến kẹt; không coi là flow khỏe |
| `CIRC_OFF_OVERRUN` | Bật lại tuần hoàn nếu an toàn; khóa AUTO 15/15 |
| `WATER_TEMP_HIGH` | Kết thúc OFF sớm; khóa dosing nếu vượt giới hạn |
| `WATER_TEMP_INVALID` | Khóa dosing và pha OFF kéo dài |
| `PH_INVALID` | Khóa pH Down |
| `EC_INVALID` | Khóa A/B |
| `BOTTLE_A_LOW` hoặc `BOTTLE_B_LOW` | Khóa cả cặp A/B |
| `BOTTLE_PH_LOW` | Khóa pH Down |
| `DOSE_TIMEOUT` | Tắt bơm, chốt fault, cộng lượng xấu nhất vào quota |
| `DOSE_INTERRUPTED` | Không chạy lại liều cũ; trộn và đo lại sau xác nhận |
| `CONFIG_INVALID` | Vào SAFE_BOOT |
| `TIME_INVALID` | Tắt lịch đèn; giữ các chức năng không cần giờ nếu an toàn |
| Mất Wi-Fi/MQTT | AUTO cục bộ tiếp tục; lưu log giới hạn |
| Watchdog/reset | Tải OFF; ghi reset reason; bắt đầu CIRC_ON mới sau tự kiểm tra |

---

## 11. Kiểm thử cảm biến bằng nước thường, chưa dùng dinh dưỡng

### 11.1. Mục đích

Giai đoạn này chỉ xác nhận cảm biến phản ứng đúng chiều và logic điều khiển chạy đúng trình tự. Đây không phải hiệu chuẩn chính xác và không dùng kết quả để châm hóa chất thật.

### 11.2. Chuẩn bị

- Bốn cốc sạch 300-500 mL.
- Nước máy để làm mẫu nền.
- Nước cất hoặc RO để pha loãng.
- Một ít giấm ăn hoặc nước cốt chanh để tạo mẫu pH thấp.
- Baking soda pha rất loãng để tạo mẫu pH cao.
- Một ít muối ăn để tạo mẫu TDS cao.
- Nước sạch để rửa đầu dò giữa các mẫu.

Không đổ các mẫu thử này vào bể 40 L và không dùng chung với cây.

### 11.3. Thử pH tăng và giảm

1. Đo cốc nước máy và ghi pH nền.
2. Cốc pH giảm:
   - thêm một giọt giấm hoặc chanh;
   - khuấy đều, chờ ổn định rồi đo;
   - thêm từng giọt nếu cần thấy xu hướng giảm rõ.
3. Cốc pH tăng:
   - pha một lượng rất nhỏ baking soda trong cốc riêng;
   - thêm dung dịch loãng từng giọt vào mẫu nước;
   - khuấy, chờ ổn định rồi xác nhận pH tăng.
4. Rửa đầu dò bằng nước sạch giữa hai cốc, không lau mạnh bóng thủy tinh.
5. Chỉ đánh giá chiều tăng/giảm và độ ổn định; không dùng giấm, chanh hoặc baking soda để hiệu chuẩn.

Lưu ý: hệ thống chỉ có bơm pH Down nên tự động chỉ giảm được pH. Khi pH quá thấp, firmware phải khóa pH Down và yêu cầu xử lý thủ công.

### 11.4. Thử TDS tăng và giảm

1. Đo nước máy làm TDS nền.
2. Cốc TDS tăng:
   - hòa tan một nhúm muối rất nhỏ hoặc khoảng 0,1 g muối trong 1 L nước;
   - khuấy tan hoàn toàn;
   - đo và xác nhận TDS/EC tăng.
3. Cốc TDS giảm:
   - lấy một phần mẫu TDS cao;
   - pha loãng dần bằng nước cất/RO;
   - đo và xác nhận TDS/EC giảm.
4. Nước máy không thể tự giảm thấp hơn nền nếu không pha loãng bằng nước có TDS thấp hơn.
5. Muối ăn chỉ dùng thử chiều phản ứng; không đại diện cho dung dịch dinh dưỡng A/B.

### 11.5. Thử logic AUTO mà vẫn chỉ bơm nước sạch

1. Các chai A, B và pH Down đều chứa nước sạch, có thể dùng màu thực phẩm khác nhau để nhìn đường ống.
2. Dùng `TEST_MODE` hoặc giá trị cảm biến giả lập để tạo các tình huống:
   - EC thấp;
   - pH cao;
   - EC cao;
   - pH thấp;
   - cảm biến `STALE/ERROR`.
3. Xác nhận:
   - EC thấp chỉ chạy A rồi B;
   - pH cao chỉ chạy pH Down;
   - EC cao hoặc pH thấp không chạy bơm;
   - dữ liệu lỗi không chạy bơm;
   - vòng 2 không vượt deadline.
4. Trên Web, nhập số giây chạy thử riêng cho A, B và pH Down; không dùng hóa chất ở bước này.
5. Đo lượng nước bơm ra bằng ống đong/cân để tính mL/s cho từng bơm tại thời gian đã nhập.
6. Nhập kết quả mL/s vào Web, lưu ngày hiệu chuẩn và thử lại ít nhất một thời lượng khác để kiểm tra tính lặp lại.

### 11.6. Hiệu chuẩn chính xác ở giai đoạn sau

- pH: dùng dung dịch chuẩn pH 4,00 và 7,00.
- EC/TDS: dùng dung dịch chuẩn đúng loại, nhiệt độ và hệ số theo tài liệu đầu dò.
- Sau hiệu chuẩn, so sánh với máy đo cầm tay độc lập trước khi cấp quyền AUTO dosing.

---

## 12. Kế hoạch chạy thử hệ thống

### Giai đoạn A - Kiểm tra logic trên máy tính

- Test debounce 2 giây.
- Test `millis()` wrap-around.
- Test deadline 15 phút.
- Test A/B/pH/refill không đồng thời.
- Test quota và timeout.
- Test reset giữa từng trạng thái.
- Test dữ liệu GOOD/STALE/ERROR/UNSTABLE.

### Giai đoạn B - Bench điện áp thấp

- Chưa cắm hóa chất và chưa cấp tải 220 V.
- Dùng LED hoặc tải giả để kiểm tra cực tính từng đầu ra.
- Thực hiện 100 lần boot/reset/brownout; không tải nào được tự bật.
- Kiểm tra E-stop và watchdog.

### Giai đoạn C - Nước sạch có cảm biến

- Bể chính dùng nước sạch.
- Bơm A/B/pH hút nước sạch.
- Thử cảm biến bằng các cốc riêng theo Chương 11.
- Đo mL/s cho từng bơm tại duty cố định.
- Thử flow, phao, refill timeout và mất điện.

### Giai đoạn D - Nghiệm thu 15/15

- Đo nước giữ lại tại đầu, giữa, cuối cả ba máng.
- Theo dõi rễ/giá thể trong suốt 15 phút OFF.
- Kiểm tra siphon, tràn, máng võng và nước chảy ngược.
- Kiểm tra flow sau mỗi lần bật.
- Kiểm tra nhiệt bơm, relay/contactor sau 48 lần đóng cắt/ngày.
- Chạy có giám sát tối thiểu 24-72 giờ bằng nước sạch và fault injection.

### Giai đoạn E - Dung dịch chuẩn và hóa chất có giám sát

- Hiệu chuẩn pH và EC/TDS.
- Thay nước trong chai test bằng dung dịch thật sau khi đã dán nhãn ống.
- Bắt đầu quota rất thấp.
- Luôn có người giám sát các liều đầu tiên.
- Chỉ mở AUTO đầy đủ khi mọi acceptance test đạt.

---

## 13. Giao diện Web mục tiêu

### Panel 1 - Cảm biến

- Hiển thị 11 kênh dữ liệu.
- Mỗi thẻ có value, unit, quality, age và trạng thái kết nối.
- Không có biểu đồ/history giả trong firmware nhúng.

### Panel 2 - Trạng thái hệ thống

- Mode hiện tại.
- State/substate hiện tại.
- Thời gian còn lại của CIRC_ON/CIRC_OFF.
- Vòng châm hiện tại và quota còn lại.
- Fault đang hoạt động và hướng dẫn xử lý.

### Panel 3 - Điều khiển

- Chọn MANUAL/AUTO/COMMISSIONING/CALIBRATION.
- Lệnh theo tên thiết bị, không hiển thị GPIO.
- Bơm định lượng dùng hold-to-run hoặc mL/thời lượng có timeout.
- Hiển thị lý do lệnh bị từ chối.

### Panel 4 - Cấu hình và bảo trì

- Chu kỳ 15/15, guard và timeout.
- Nhóm cấu hình timeline theo giây: `quiet_settle_before_m0_s`, `measure_m0_max_s`, `dose_round1_max_s`, `mix_round1_s`, `settle_round1_s`, `measure_m1_max_s`, `dose_round2_max_s`, `mix_round2_s`, `settle_round2_s`, `measure_m2_max_s`, `ab_separation_mix_s` và `safety_margin_s`.
- Hiển thị `timeline_total_s`, `off_remaining_s` và cảnh báo ngay khi cấu hình không đủ chỗ cho một hoặc hai vòng.
- Cấu hình số giây chạy thử từng bơm bằng nước; sau hiệu chuẩn hiển thị mL/s và thời gian chạy thực tế được tính toán.
- Ngưỡng EC25, pH, nhiệt độ và flow.
- Calibration bơm/cảm biến, quota và ngày hết hạn.
- Wi-Fi, MQTT, RTC/NTP, phiên bản firmware và OTA.

### 13.5. Quy chuẩn JSON Payload và Ánh xạ NVS Storage Key

#### Quy chuẩn NVS Storage Key (ESP-IDF giới hạn tối đa 15 ký tự/key)

| Tham số Cấu hình | NVS Namespace | NVS Key ($\le 15$ ký tự) | Kiểu dữ liệu | Giá trị mặc định |
|---|---|---|---|---|
| Thời gian ON tuần hoàn | `cfg_timer` | `circ_on_s` | uint16 | 900 |
| Thời gian OFF tuần hoàn | `cfg_timer` | `circ_off_s` | uint16 | 900 |
| Chờ tĩnh trước M0 | `cfg_timeline` | `t_settle0` | uint16 | 30 |
| Đo M0 | `cfg_timeline` | `t_meas0` | uint16 | 60 |
| Châm vòng 1 | `cfg_timeline` | `t_dose1` | uint16 | 120 |
| Sục trộn vòng 1 | `cfg_timeline` | `t_mix1` | uint16 | 180 |
| Chờ tĩnh trước M1 | `cfg_timeline` | `t_settle1` | uint16 | 30 |
| Đo M1 | `cfg_timeline` | `t_meas1` | uint16 | 60 |
| Châm vòng 2 | `cfg_timeline` | `t_dose2` | uint16 | 60 |
| Sục trộn vòng 2 | `cfg_timeline` | `t_mix2` | uint16 | 120 |
| Chờ tĩnh trước M2 | `cfg_timeline` | `t_settle2` | uint16 | 30 |
| Đo M2 | `cfg_timeline` | `t_meas2` | uint16 | 60 |
| Sục trộn giữa A và B | `cfg_timeline` | `t_ab_sep` | uint16 | 45 |
| Lề an toàn kết thúc | `cfg_timeline` | `t_safe_margin` | uint16 | 60 |
| Tỷ lệ lưu lượng bơm A (mL/s) | `cfg_calib` | `flow_pump_a` | float | 1.25 |
| Tỷ lệ lưu lượng bơm B (mL/s) | `cfg_calib` | `flow_pump_b` | float | 1.25 |
| Tỷ lệ lưu lượng bơm pH (mL/s) | `cfg_calib` | `flow_pump_ph` | float | 0.85 |

#### Mẫu JSON Command gửi từ Web/MQTT (`CommandQueue`)

```json
{
  "cmd_id": "REQ-10023",
  "sender": "web_admin",
  "action": "MANUAL_CONTROL",
  "target": "BOMLL1",
  "val": 100,
  "duration_ms": 5000,
  "ttl_s": 30
}
```

#### Mẫu JSON Telemetry gửi về Web Client / MQTT Broker

```json
{
  "sys_mode": "AUTO",
  "sys_state": "CIRC_OFF",
  "sub_state": "QUIET_MEASURE",
  "off_remaining_s": 420,
  "timeline_total_s": 770,
  "active_fault": "NONE",
  "sensors": {
    "temp_water": {"val": 24.2, "unit": "C", "qual": "GOOD", "age_ms": 1200},
    "ph": {"val": 6.15, "unit": "pH", "qual": "GOOD", "age_ms": 500},
    "ec25": {"val": 1.42, "unit": "mS/cm", "qual": "GOOD", "age_ms": 500},
    "flow": {"val": 0.0, "unit": "L/min", "qual": "GOOD", "age_ms": 200}
  }
}
```

---

## 14. Lộ trình lập trình ngày mai

### P0 - Nền tảng và an toàn đầu ra

1. Chốt board profile N16R8 và partition 16 MB có OTA.
2. Đặt tên GPIO theo nghiệp vụ.
3. Viết HAL input/output.
4. Viết `output_arbiter` và safe boot OFF.
5. Thêm hard timeout cho bơm.
6. Viết test cực tính trên tải giả.

### P1 - Domain và cấu hình

1. `SensorSample`, `Command`, `SystemState`, `FaultCode`.
2. `ControlConfig` version + CRC + range check.
3. Calibration và quota.
4. Event journal và `dose_in_progress`.
5. Fake clock để unit test deadline.

### P2 - Cảm biến

1. Phao + debounce.
2. Flow + đếm xung nguyên tử.
3. SHT31/BH1750.
4. DS18B20 không blocking.
5. pH và EC/TDS với lọc, quality và calibration.

### P3 - FreeRTOS và FSM

1. Queue và heartbeat.
2. `TaskSensors`.
3. `TaskSafetyControl`.
4. CIRC_ON/CIRC_OFF 15/15.
5. Flow retry và refill.
6. Quiet Window.
7. DOSE_AB, DOSE_PH, MIX và hai vòng có deadline.
8. Fault matrix và recovery.

### P4 - Web/MQTT/OTA

1. API lệnh nghiệp vụ.
2. TTL, dedup và ACK.
3. Telemetry gồm state, quality, age, quota và fault.
4. Form cấu hình timeline theo giây, kiểm tra tổng thời gian và lưu cấu hình có version/CRC.
5. Xác thực giao diện.
6. OTA an toàn và rollback.

### P5 - Kiểm thử tăng dần

1. Unit test trên PC.
2. Bench điện áp thấp.
3. Nước sạch + sensor test cups.
4. Chạy 15/15 trong 24-72 giờ.
5. Dung dịch chuẩn.
6. Hóa chất có giám sát.
7. AUTO quota thấp rồi mới tăng dần.

---

## 15. Tiêu chí nghiệm thu bắt buộc

- [ ] N16R8 nhận đúng 16 MB Flash và 8 MB PSRAM.
- [ ] OTA image A/B và rollback hoạt động.
- [ ] 100 lần reset/brownout không tự bật tải.
- [ ] CIRC_ON chỉ đếm sau khi flow hợp lệ.
- [ ] CIRC_OFF không vượt 900 giây ngoài guard.
- [ ] Nước còn 1-2 cm tại đầu, giữa và cuối từng máng.
- [ ] Không có siphon, tràn hoặc điểm khô trong 15 phút OFF.
- [ ] Flow thấp retry đúng số lần rồi latch fault.
- [ ] Flow 0 khi bơm OFF không tạo lỗi giả.
- [ ] A, B, pH và refill không bao giờ chạy chồng nhau.
- [ ] Timeout cứng tắt được từng bơm.
- [ ] Quota theo lần/giờ/ngày hoạt động.
- [ ] Reset giữa liều tạo `DOSE_INTERRUPTED` và không châm tiếp.
- [ ] Web cho phép nhập các thời gian châm/trộn/chờ/đo theo giây và hiển thị tổng timeline.
- [ ] Cấu hình vượt `circ_off_s` bị cảnh báo; vòng 2 hoặc toàn bộ lần châm được hoãn an toàn.
- [ ] Không còn giả định cố định 300/150 giây khi chưa có kết quả thử nước và đo mL/s.
- [ ] pH/EC `STALE/ERROR/UNSTABLE` khóa dosing.
- [ ] Test giấm/chanh làm pH giảm và baking soda làm pH tăng trong cốc riêng.
- [ ] Test muối làm TDS tăng và nước RO/cất làm TDS giảm.
- [ ] Mất Wi-Fi/MQTT không dừng AUTO cục bộ.
- [ ] E-stop, watchdog, refill timeout và overflow đều đưa tải về trạng thái an toàn.
- [ ] Chạy nước sạch 24-72 giờ với fault injection không có lỗi nguy hiểm.

---

## 16. Quy trình trồng xà lách mỡ tham khảo

### Giai đoạn 1 - Nảy mầm

- Gieo vào rockwool/xơ dừa sạch đã làm ẩm.
- Giữ ẩm nhưng không ngập bí.
- Theo dõi nảy mầm và loại cây yếu.

### Giai đoạn 2 - Cây con

- Đưa ra ánh sáng phù hợp.
- Dùng nước sạch hoặc dinh dưỡng loãng theo sản phẩm.
- Chỉ chuyển lên máng khi rễ và cây đủ khỏe.

### Giai đoạn 3 - Làm quen máng

- Chạy tuần hoàn có giám sát để rễ chạm được vùng nước.
- Khi lứa cây chưa đủ rễ, chưa mở 15 phút OFF tự động.
- Thực hiện `trial_circ_off` một lần và quan sát toàn bộ cây.

### Giai đoạn 4 - AUTO 15/15

- Chỉ bật sau khi rễ/giá thể tiếp xúc được lớp nước giữ lại.
- Theo dõi héo, nhiệt độ nước, màu rễ và flow hàng ngày.
- Mục tiêu EC/pH phải theo giống, giai đoạn, nước nguồn và nhãn dinh dưỡng; không dùng một số cố định cho mọi lứa.

### Giai đoạn 5 - Thu hoạch và vệ sinh

- Thu hoạch theo kích thước và chất lượng thực tế.
- Vệ sinh bể, ống, máng và đầu lọc giữa các lứa.
- Lứa mới phải xác nhận lại readiness cho chế độ 15/15.

---

## 17. Tài liệu tham khảo

1. Espressif, ESP32-S3-WROOM-1/WROOM-1U Datasheet: <https://documentation.espressif.com/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf>
2. Virginia Cooperative Extension, NFT Systems: <https://www.pubs.ext.vt.edu/SPES/spes-463/spes-463.html>
3. Oregon State University Extension, Nutrient Film Technique: <https://extension.oregonstate.edu/catalog/pub/em-9457-hydro-hints-nutrient-film-technique>
4. University of Minnesota Extension, Small-scale Hydroponics: <https://extension.umn.edu/gardening-minnesota/small-scale-hydroponics>
5. Oklahoma State University Extension, EC and pH Guide: <https://extension.okstate.edu/fact-sheets/electrical-conductivity-and-ph-guide-for-hydroponics>
6. Iowa State University Extension, Supplemental Light: <https://yardandgarden.extension.iastate.edu/how-to/growing-indoor-plants-under-supplemental-lights/important-considerations-providing-supplemental-light-indoor-plants>

---

## 18. Tóm tắt quy tắc cốt lõi

1. Hệ thống vận hành theo 15 phút tuần hoàn và 15 phút nghỉ sau khi đã nghiệm thu máng và lứa cây.
2. Chỉ một khối điều khiển được phép ghi đầu ra.
3. Đo hợp lệ trước, châm theo mL đã hiệu chuẩn sau.
4. A, B, pH Down và refill không chạy chồng nhau.
5. Thời gian châm, trộn, chờ tĩnh và đo được cấu hình theo giây trên Web; không chốt cứng 5 phút khi chưa thử thực tế.
6. Vòng 2 là micro-dose có timeline riêng; không đủ thời gian theo cấu hình thì hoãn.
7. pH thấp và EC cao không có cơ cấu tự sửa trong phần cứng hiện tại, phải báo người vận hành.
8. Nghi ngờ hoặc lỗi dữ liệu thì tắt bơm hóa chất, giữ cây an toàn và yêu cầu kiểm tra.
