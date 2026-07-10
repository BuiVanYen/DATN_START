# ✅ PCB DESIGN CHECKLIST — ĐỒ ÁN THỦY CANH NFT

> **MCU:** ESP32-S3-WROOM-1-N8R8
> **Nguồn:** Tổ ong 12V 20A → LM2596 5V → AMS1117 3.3V
> **Nạp code:** UART (không dùng USB-C/CH340C)
> **Module ngoài:** Module pH (PH0-14 Đức Huy) + Module TDS (có tích hợp DS18B20)
> **Cập nhật:** 2026-06-10

---

## 📊 TỔNG QUAN TRẠNG THÁI

| Khối | Trạng thái | Ghi chú |
|---|---|---|
| ① Nguồn 12V→5V (LM2596) | ✅ XONG | Đã thiết kế |
| ② ESP32-S3 cơ bản + UART | ✅ XONG | Nạp qua UART, không cần CH340C/USB-C |
| ③ AMS1117 3.3V | ✅ XONG | Đã thiết kế (đã kiểm tra schematic_hien_tai.pdf) |
| ④ PWM MOSFET (6 kênh) | ✅ XONG | 2× Quạt Tầng + 3× Bơm DD + 1× Bơm 12V |
| ⑤ Relay (4 kênh) | ✅ XONG | 2× Đèn Tầng + 1× Bơm 220V + 1× Sục 220V |
| ⑥ Header cảm biến | ✅ XONG | I2C, pH, TDS, Y26/Level ×4 |
| ⑦ Buzzer + LED | ✅ XONG | Tích hợp LED_SYS & Buzzer đệm dòng |
| ⑧ GPIO Assignment | ✅ XONG | Đã gán chân tối ưu không xung đột |
| ⑨ Layout & Routing | ⬜ CHƯA | Sắp xếp linh kiện + đi dây đồng |
| ⑩ DRC/ERC & Gerber | ⬜ CHƯA | Kiểm tra cuối |

---

## ĐÃ HOÀN THÀNH ✅

### ① Khối Nguồn — LM2596 5V
- [x] Mạch LM2596 hạ áp 12V → 5V
- [x] Tụ lọc input/output
- [x] Cuộn cảm + diode Schottky
- [x] Terminal đầu vào 12V

### ② Khối MCU — ESP32-S3-WROOM-1-N8R8
- [x] Footprint ESP32-S3-WROOM-1-N8R8 (SMD)
- [x] Nút RESET (EN pin + RC reset)
- [x] Nút BOOT (GPIO0)
- [x] Header UART (TX/RX/GND) để nạp code
- [x] Bypass capacitor 0.1µF + 10µF cho VCC

### ③ Khối Nguồn 3.3V — AMS1117-3.3
- [x] Mạch hạ áp 5V → 3.3V dùng AMS1117-3.3 (SOT-223)
- [x] Tụ input/output (10µF/22µF) lọc nguồn nhiễu
- [x] LED báo nguồn 3.3V
- [x] Đảm bảo dòng đủ cấp cho ESP32-S3 (peak 500mA, LDO chịu được 800mA)

---

**Phân phối nguồn:**
| Rail | Cấp cho |
|---|---|
| 12V | MOSFET drain (bơm, quạt), Relay coil, Y26 sensor |
| 5V | Module pH, Module TDS, Relay logic (PC817), Buzzer |
| 3.3V | ESP32-S3, I2C sensors (SHT31, BH1750) |

---

## CÒN LÀM ⬜

### ④ Khối PWM MOSFET — IRLZ44N (6-7 kênh)
> Ưu tiên: 🔴 CAO

**Mỗi kênh MOSFET gồm:**
- [x] AO3400A (SOT-23, N-Channel, Vgs(th)=0.65-1.45V)
- [x] R10kΩ nối tiếp từ GPIO → Gate (hạn dòng, chống nhiễu ngõ ra) [Cập nhật theo ý kiến sếp]
- [x] Loại bỏ trở pull-down Gate xuống GND (không cần trở xuống mass) [Cập nhật theo ý kiến sếp]
- [x] 1N5819 Schottky / SS14 flyback diode (Cathode→+12V, Anode→Drain)
- [x] Terminal KF301-2P cho mỗi tải (+12V, DRAIN)

**Danh sách kênh PWM:**

| Kênh | Tải | GPIO | Ghi chú |
|---|---|---|---|
| PWM1 | 2× Quạt Tầng 1 (mắc song song) | GPIO11 | Chân 19 (Cạnh Dưới) |
| PWM2 | 2× Quạt Tầng 2 (mắc song song) | GPIO10 | Chân 18 (Cạnh Dưới) |
| PWM3 | Bơm DD chai A (pH Up) | GPIO13 | Chân 21 (Cạnh Dưới) |
| PWM4 | Bơm DD chai B (Nutrient A) | GPIO12 | Chân 20 (Cạnh Dưới) |
| PWM5 | Bơm DD chai C (Nutrient B) | GPIO3 | Chân 15 (Cạnh Dưới) - Lưu ý chân Strapping |
| PWM6 | Bơm nước 12V DC chính | GPIO9 | Chân 17 (Cạnh Dưới) |

> **ESP32-S3-WROOM-N8R8** có Octal PSRAM/Flash → **TRÁNH dùng GPIO 26-37** (bị reserve).
> Các GPIO an toàn cho PWM: 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 21, 38, 39, 40, 41, 42, 47, 48
> GPIO 0: Strapping pin (BOOT) — tránh dùng cho output
> GPIO 19, 20: USB D-/D+ — tránh nếu không dùng USB
> GPIO 43, 44: UART0 TX/RX mặc định — dùng cho nạp code

**Checklist vẽ schematic:**
- [x] Vẽ 1 kênh MOSFET mẫu → copy 6-7 lần
- [x] Gán GPIO cho từng kênh (tránh strapping pins)
- [x] Đặt net label rõ ràng: PWM_PUMP_A, PWM_PUMP_B, PWM_FAN1-4
- [x] Kiểm tra tổng dòng 12V rail: 2+2+0.5×4 = 6A (OK với nguồn 20A)

---

### ⑤ Khối Relay — PC817 + S8050 + SRD-12VDC (6 kênh)
> Ưu tiên: 🔴 CAO

**Mỗi kênh Relay gồm:**
- [x] ESP GPIO → R1kΩ → PC817 LED anode (cách ly quang)
- [x] PC817 phototransistor → S8050 driver
- [x] S8050 Collector → Relay Coil(-), Coil(+) → +12V
- [x] 1N4148 flyback diode qua relay coil
- [x] LED 3mm báo trạng thái (qua R1kΩ)
- [x] Terminal cho output relay

**Danh sách kênh Relay:**

| Kênh | Tải | GPIO | Terminal |
|---|---|---|---|
| RL1 | Đèn LED Tầng 1 (2 đèn song song) | GPIO17 | 2P (Nối sang ULN2803 ngõ vào I1) |
| RL2 | Đèn LED Tầng 2 (2 đèn song song) | GPIO18 | 2P (Nối sang ULN2803 ngõ vào I2) |
| RL3 | Bơm chìm 220V AC chính | GPIO6 | 2P (Nối sang ULN2803 ngõ vào I3) |
| RL4 | Bơm sủi khí oxy 220V AC | GPIO7 | 2P (Nối sang ULN2803 ngõ vào I4) |

**Checklist vẽ schematic:**
- [x] PC817 (DIP-4) ×6 gói (6 kênh riêng biệt)
- [x] S8050 NPN ×6 (SOT-23 hoặc TO-92)
- [x] SRD-12VDC-SL-C relay ×6
- [x] Khu vực 220V: rãnh cách ly PCB ≥ 3mm, khoảng cách clearance ≥ 3mm (cho Bơm chính & Bơm sục)
- [x] Terminal 3P cho 2 tải 220V, terminal 2P cho 4 tải đèn 12V

---

### ⑥ Khối Header Cảm Biến
> Ưu tiên: 🟡 TRUNG BÌNH

#### 6a. I2C Header (SHT31 + BH1750)
- [x] Header 4P: 3.3V, GND, SDA, SCL
- [x] Pull-up 4.7kΩ ×2 (SDA, SCL lên 3.3V)
- [x] Bypass 0.1µF gần header

#### 6b. Module TDS (tích hợp DS18B20)
- [x] Header 3P cho module TDS: 5V, GND, SIG_TDS
- [x] Header riêng cho DS18B20: 5V_TDS (cấp nguồn qua P-MOSFET Q8), GND, DATA (DS18B20)
- [x] Cầu phân áp TDS output (5V→3V): R10k + R15k
- [x] Mạch lọc RC: R10kΩ + C100nF trước GPIO ADC (R19 + C9) [Tăng trở kháng ngõ vào theo sếp]
- [x] Pull-up 4.7kΩ cho DS18B20 OneWire DATA (R20) (không lắp tụ lọc tránh méo xung OneWire)

> **Lưu ý:** Module TDS đã được kết nối thông qua mạch đóng ngắt nguồn P-MOSFET Q8 để tiết kiệm điện và chống điện phân đầu dò.

#### 6c. Module pH (PH0-14 Đức Huy)
- [x] Header 3P: 5V, GND, SIG_pH
- [x] Cầu phân áp pH output (5V→3.3V): R10k + R15k (R16 + R17)
- [x] Mạch lọc RC: R10kΩ + C100nF trước GPIO ADC (R18 + C8) [Tăng trở kháng ngõ vào theo sếp]

#### 6d. Cảm biến Mực Nước Không Tiếp Xúc ×4
- [x] Dùng cho: 1 Thùng chứa nước chính + 3 Chai dung dịch (A, B, pH Down)
- [x] Header 3P ×4: 5V, GND, SIG (LEVEL1, LEVEL2, LEVEL3, LEVEL4)
- [x] Mạch phân áp 5V → 3.3V: R10k + R15k cho mỗi cảm biến để bảo vệ GPIO ESP32
- [x] Tụ lọc nhiễu 100nF song song trở 15kΩ xuống GND (C_LV1-C_LV4) [Chống nhiễu kéo dây xa theo sếp]

#### 6e. Cảm biến Lưu lượng nước (Flow Sensor)
- [x] Header 3P: 5V, GND, SIG_FLOW
- [x] Cầu phân áp bảo vệ (5V→3.3V): R10kΩ + R15kΩ (R32 + R31)
- [x] Tụ lọc nhiễu 100nF song song trở R31 xuống GND (C_FLOW) [Chống nhiễu kéo dây xa theo sếp]

**Tổng GPIO cảm biến:**
| Cảm biến | GPIO | Giao tiếp | Ghi chú |
|---|---|---|---|
| SDA (I2C) | GPIO5 | I2C | SHT31, BH1750 (Chân 5) |
| SCL (I2C) | GPIO4 | I2C | SHT31, BH1750 (Chân 4) |
| DS18B20 | GPIO14 | OneWire | Cảm biến nhiệt độ TDS (Chân 22) |
| TDS analog | GPIO2 | ADC1_CH1 | Chân 38 (Cạnh Phải), ADC1 an toàn với WiFi |
| EN_TDS | GPIO42 | Digital Out | Chân 35 (Cạnh Phải), điều khiển nguồn TDS |
| pH analog | GPIO1 | ADC1_CH0 | Chân 39 (Cạnh Phải), ADC1 an toàn với WiFi |
| Lưu lượng nước | GPIO41 | Digital Input | Chân 34 (Cạnh Phải), Flow Sensor |
| Mực nước Thùng | GPIO40 | Digital Input | Chân 33 (Cạnh Phải), LEVEL1 |
| Mực nước Chai 1 | GPIO39 | Digital Input | Chân 32 (Cạnh Phải), LEVEL2 |
| Mực nước Chai 2 | GPIO38 | Digital Input | Chân 31 (Cạnh Phải), LEVEL3 |
| Mực nước Chai 3 | GPIO21 | Digital Input | Chân 23 (Cạnh Dưới), LEVEL4 |

---

### ⑦ Buzzer + LED Hệ Thống
> Ưu tiên: 🟢 THẤP

- [x] Buzzer 5V + AO3400A N-MOSFET driver + R100Ω gate + R10kΩ pull-down + Diode SS14 bảo vệ
- [x] LED báo nguồn 3.3V / PWR (qua R1kΩ)
- [x] LED báo trạng thái hệ thống LED_SYS (qua R1kΩ)

---

---

### ⑧ GPIO Assignment — ESP32-S3-WROOM-N8R8
> Ưu tiên: 🔴 CAO — Phải làm trước khi routing

> **CÁC GPIO BỊ CẤM (Octal PSRAM N8R8):**
> - GPIO 26-32: Flash/PSRAM data lines
> - GPIO 33-37: Flash/PSRAM control lines
> - GPIO 0: Strapping BOOT — không dùng làm output thường
> - GPIO 3: Strapping JTAG — cẩn thận
> - GPIO 19, 20: USB D-/D+ (nếu không dùng native USB)
> - GPIO 43: UART0 TX (dùng cho nạp code)
> - GPIO 44: UART0 RX (dùng cho nạp code)
> - GPIO 45: Strapping VDD_SPI
> - GPIO 46: Strapping log output

**Bước làm:**
- [x] Lập bảng GPIO đầy đủ tối ưu theo chiều xoay 180 độ
- [x] Phân chia: ADC1 → cảm biến analog, Digital → Y26, PWM → MOSFET, Digital Out → Relay
- [x] Kiểm tra ADC: ESP32-S3 dùng ADC1 (GPIO 1, 2) cho pH và TDS (không bị WiFi ảnh hưởng)
- [x] I2C đã đổi: SDA=GPIO5, SCL=GPIO4
- [x] Cập nhật lại bảng GPIO trong plan chính (đã gán)

---

### ⑨ Layout & Routing PCB
> Ưu tiên: 🟡 SAU KHI SCHEMATIC XONG

- [ ] Kích thước board: ~180×120mm (hoặc tối ưu lại)
- [ ] 2 lớp (Top + Bottom)
- [ ] Phân vùng:
  - Nguồn (LM2596 + AMS1117) — góc trái trên
  - MCU (ESP32 + UART header) — giữa trên
  - Cảm biến (header) — phải trên
  - MOSFET (TO-220) — giữa dưới
  - Relay + 220V — phải dưới (cách ly)
- [ ] Anten ESP32 hướng rìa board, KHÔNG đổ GND plane bên dưới anten
- [ ] Đường 12V ≥ 2mm width
- [ ] Đường signal ≥ 0.3mm
- [ ] GND plane lớp dưới (pour)
- [ ] Rãnh cách ly 220V ≥ 3mm
- [ ] MOSFET TO-220 cần pad tản nhiệt
- [ ] Lỗ bắt vít 4 góc (M3)

---

### ⑩ Kiểm Tra & Xuất File
> Ưu tiên: SAU LAYOUT

- [ ] Chạy ERC (Electrical Rules Check) → fix hết error
- [ ] Chạy DRC (Design Rules Check) → fix hết error
- [ ] Review lại net label, footprint
- [ ] Xuất file Gerber
- [ ] Xuất file BOM
- [ ] Đặt mạch JLCPCB / PCBWay

---

## 📝 SO SÁNH ĐỒ ÁN CŨ vs MỚI

| Hạng mục | Đồ án CŨ (BOM cũ) | Đồ án MỚI |
|---|---|---|
| MCU | ESP32S NodeMCU LuaNode32 CH340C | ESP32-S3-WROOM-N8R8 (SMD trên PCB) |
| Nạp code | USB trên DevKit | UART header (TX/RX/GND) |
| Mạch | Module rời trên board đục lỗ | PCB custom tích hợp |
| Nguồn 5V | Adapter 5V-10A riêng | LM2596 hạ từ 12V (trên PCB) |
| Nguồn 3.3V | Từ DevKit onboard | AMS1117-3.3 (trên PCB) |
| Chai DD | 3 chai + 4 bơm peristaltic | 2 chai AB + 2 bơm peristaltic |
| Bơm sục | Máy sủi khí Resun 220V AC | Máy sủi khí 220V AC (Relay) |
| Bơm chìm | 1× bơm chìm 12V DC | Không dùng |
| Bơm 220V | 1× bơm chìm 220V AC | 1× bơm 220V AC (relay) |
| Quạt | 3× quạt 12V DC 120mm | 4× quạt 12V (2/tầng) |
| Đèn | Không có trong BOM cũ | 4× đèn LED 12V (2/tầng) |
| Cảm biến mực | 5× cảm biến mức chất lỏng | 4× Y26 PNP |
| DS18B20 | Không rõ (có thể chung SHT30) | Tích hợp trong module TDS |
| RF | Module CC1101 + Antenna | Không dùng (WiFi/MQTT) |
| Pin NLMT | Tấm pin 100W + ắc quy Delkor | Không dùng |
| Driver motor | 2× L298N | IRLZ44N MOSFET (trên PCB) |
| Relay | Module relay 4CH (xanh) | SRD-12VDC + PC817 (trên PCB) |

---

## 🔄 THỨ TỰ LÀM TIẾP (ĐỀ XUẤT)

```
1. ⑧ Phân bổ GPIO        → quyết định trước khi vẽ các khối
2. ④ PWM MOSFET 6-7 kênh → bơm DD + quạt
3. ⑤ Relay 6 kênh         → bơm 220V + sục 220V + đèn 12V
4. ⑥ Header cảm biến      → pH, TDS, I2C, Y26
5. ⑦ Buzzer/LED           → nếu cần
6. ⑨ Layout & Routing     → phân vùng + routing
7. ⑩ DRC/ERC → Gerber     → đặt mạch
```
