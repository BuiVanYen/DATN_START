# 📐 HƯỚNG DẪN CHI TIẾT LAYOUT PCB & CHỐNG NHIỄU (ĐỒ ÁN THỦY CANH NFT)

Tài liệu này cung cấp các thông số kỹ thuật chi tiết nhất về **kích thước bo mạch, cách bố trí linh kiện (Floorplanning), và các nguyên lý đi dây chống nhiễu** cho bo mạch điều khiển sử dụng chip **ESP32-S3-WROOM-1-N8R8**.

---

## 1. Kích thước Bo mạch (Board Dimensions)

*   **Kích thước khuyến nghị:** **$150\text{ mm} \times 100\text{ mm}$** (Tỉ lệ chuẩn 3:2).
    *   *Lý do:* Kích thước này đủ rộng để bố trí rãnh cách ly an toàn giữa phần điện 220V và phần xử lý tín hiệu yếu, đồng thời khớp hoàn hảo với các hộp nhựa chống nước thông dụng trên thị trường (Ví dụ: hộp nhựa $160 \times 110 \times 90\text{ mm}$).
*   **Số lớp đồng (Layers):** **2 lớp (Double-sided PCB)**:
    *   `Top Layer` (Lớp trên): Chủ yếu chạy dây tín hiệu, đặt linh kiện SMD và các đường nguồn chính.
    *   `Bottom Layer` (Lớp dưới): Chủ yếu làm mặt phủ đồng tiếp địa (GND Plane) để chống nhiễu và chạy một số đường dây phụ chéo nhau.
*   **Lỗ bắt vít định vị:** **4 lỗ M3** đường kính lỗ **$3.2\text{ mm}$** đặt tại 4 góc. Tâm lỗ cách mép bo mạch **$5\text{ mm}$**. Bo tròn 4 góc mạch với bán kính $R = 3\text{ mm}$ để bo mạch không bị sắc cạnh.

---

## 2. Bản đồ Bố trí Linh kiện (Component Placement / Floorplan)

Để tránh nhiễu chéo từ các linh kiện công suất cao sang linh kiện xử lý tín hiệu nhỏ, bo mạch được chia thành **5 phân khu chức năng** như sơ đồ khối bên dưới:

```text
+-----------------------------------------------------------------------------------+
| [KHU 1: NGUỒN HẠ ÁP]                     | [KHU 2: VI XỬ LÝ (MCU)]                |
| - Jack DC / Terminal 12V                 | - Module ESP32-S3 (Ăng-ten quay lên)   |
| - Cầu chì F1                             | - Phím nhấn RESET & BOOT               |
| - Buck LM2596 (5V) & LDO AMS1117 (3.3V)  | - Header nạp code UART (J1)            |
|                                          +----------------------------------------|
|                                          | [KHU 3: CẢM BIẾN TÍN HIỆU YẾU]         |
|                                          | - Đầu cắm pH, TDS, I2C, Mực nước       |
|                                          | - Cầu phân áp & Tụ lọc nhiễu RC        |
+------------------------------------------+----------------------------------------|
| [KHU 4: CÔNG SUẤT PWM 12V]               | [KHU 5: ĐIỆN ÁP CAO 220V (RELAY)]      |
| - 7x MOSFET AO3400A (Dạng dán SOT-23)    | - 6x Relay SRD-12VDC                   |
| - 7x Diode dập xung SS14                 | - IC đệm dòng ULN2803                  |
| - 7x LED báo trạng thái                  | - 6x Terminal đầu ra 220V              |
| - 7x Terminal ngõ ra PWM (Quạt/Bơm)      |                                        |
|                                          | ======= RÃNH CÁCH LY 3MM (CUTOUT) ==== |
|                                          | [Terminal 220V AC Output]              |
+-----------------------------------------------------------------------------------+
```

### Hướng dẫn đặt linh kiện chi tiết:

1.  **Module ESP32-S3 (Khu 2):**
    *   Đặt ở mép trên của bo mạch. Phần **Ăng-ten PCB (vùng gạch chéo trên module)** phải **chìa ra ngoài mép bo mạch** hoặc vùng PCB bên dưới nó phải trống hoàn toàn (không phủ đồng, không đi dây ở cả 2 lớp).
2.  **Khối Relay & Đường AC 220V (Khu 5):**
    *   Đặt toàn bộ 6 Relay thành 1 hàng dọc dọc theo mép phải bo mạch.
    *   Các Terminal ngõ ra của Relay phải đặt hướng ra ngoài rìa.
    *   **Rãnh cách ly (Board Cutout):** Vẽ một rãnh trống không có đồng, chiều rộng **$3.0\text{ mm}$** nằm ngay dưới thân của Relay (ngăn đôi giữa chân cuộn hút kích và các chân tiếp điểm COM/NO/NC).
3.  **Tụ lọc nguồn & Tụ lọc nhiễu tín hiệu (Capacitors):**
    *   Các tụ lọc nguồn gốm $100\text{ nF}$ (`C4`, `C5`) cho ESP32 bắt buộc phải đặt sát chân cấp nguồn (trong khoảng cách dưới $5\text{ mm}$).
    *   Các tụ lọc nhiễu tín hiệu cảm biến (`C8`, `C9` cho pH/TDS; `C_LV1-C_LV4`, `C_FLOW` cho mực nước và lưu lượng) **bắt buộc phải đặt sát chân vi điều khiển ESP32 nhất có thể** (dưới $5\text{ mm}$), không đặt gần connector cảm biến. Việc này đảm bảo triệt tiêu toàn bộ nhiễu từ các đường dây cảm biến kéo dài trước khi tín hiệu đi vào chân chip.

---

## 3. Quy tắc Thiết lập Độ rộng Đường mạch (Routing Trace Width)

Độ rộng đường dây đồng (Trace Width) được tính toán dựa trên dòng điện đi qua để tránh nóng mạch và sụt áp:

| Đường mạch | Dòng điện tối đa | Độ rộng tối thiểu (mm) | Độ rộng khuyến nghị (mil) | Lớp đồng (Layer) |
| :--- | :--- | :--- | :--- | :--- |
| **Nguồn vào 12V chính** | $\sim 5\text{ A} - 6\text{ A}$ | $2.5\text{ mm}$ | $100\text{ mil}$ | Top / Bottom |
| **Đường AC 220V của Relay** | $\sim 10\text{ A}$ | $3.0\text{ mm}$ (hoặc phủ thiếc) | $120\text{ mil}$ | Top & Bottom |
| **Đường ra tải MOSFET (12V)**| $\sim 1\text{ A} - 2\text{ A}$ | $1.2\text{ mm}$ | $48\text{ mil}$ | Top |
| **Đường cấp nguồn 5V** | $\sim 1\text{ A}$ | $0.8\text{ mm}$ | $32\text{ mil}$ | Top |
| **Đường cấp nguồn 3.3V** | $\sim 0.5\text{ A}$ | $0.6\text{ mm}$ | $24\text{ mil}$ | Top |
| **Đường tín hiệu điều khiển** | $< 0.02\text{ A}$ | $0.254\text{ mm}$ | $10\text{ mil}$ | Top / Bottom |
| **Tín hiệu Analog (pH, TDS)** | Cực nhỏ | $0.3\text{ mm}$ | $12\text{ mil}$ | Top (chạy ngắn nhất) |

> [!TIP]
> **Mẹo tăng dòng cho tiếp điểm Relay:** Do mạch chỉ có 2 lớp đồng thông thường ($1\text{ oz}$ độ dày), đối với các đường mạch 220V chạy ra tải lớn, bạn nên **để lộ đồng không phủ sơn bảo vệ (Solder Mask Open)** trên các đường mạch này để sau khi gia công có thể đắp thêm một lớp thiếc (solder) lên trên nhằm tăng độ dày dẫn điện.

---

## 4. Thiết kế Chống nhiễu Chi tiết (Anti-Interference Layout)

Đặc thù đồ án thủy canh có cả bơm nước 220V (gây nhiễu xung cảm kháng rất lớn khi đóng cắt) và cảm biến pH/TDS (tín hiệu Analog siêu nhạy, dễ bị sai số). Bạn cần tuân thủ tuyệt đối các quy tắc chống nhiễu sau:

### 4.1. Chống nhiễu cho Khối Cảm biến Analog (pH & TDS)
*   **Đường chạy tín hiệu cực ngắn:** Đặt mạch phân áp ($R_{10k}/R_{15k}$) và mạch lọc RC ($R_{1k}/C_{100nF}$) **gần sát chân ADC của ESP32 nhất có thể**, không đặt gần các connector cảm biến. Việc này giúp tín hiệu truyền từ connector về đến mạch lọc là tín hiệu 5V dòng lớn hơn (ít nhiễu hơn), sau đó mới lọc và hạ áp ngay tại chân chip.
*   **Dây bọc chống nhiễu (Guard Trace):** Bao quanh đường mạch tín hiệu pH (`ADC_PH`) và TDS (`ADC_TDS`) bằng một đường mạch `GND` chạy song song hai bên để triệt tiêu nhiễu điện trường xung quanh.
*   **Tránh xa nguồn nhiễu:** Không chạy đường tín hiệu pH và TDS song song hoặc nằm gần đường mạch xung PWM điều khiển quạt/bơm của MOSFET và đường 220V của Relay.

### 4.2. Chống nhiễu Xung ngược từ Tải cảm ứng (Bơm, Quạt, Relay)
*   Khi động cơ bơm 12V hoặc cuộn hút Relay ngắt điện, cuộn dây bên trong sẽ tạo ra xung điện áp ngược hàng trăm Volt.
*   **Giải pháp trên mạch:** Đặt các diode dập xung ngược `SS14` (của MOSFET) và `1N4148` (của Relay) **sát ngay cạnh hai chân của connector/cuộn hút**. Xung ngược sẽ bị tiêu tán ngay tại diode, không truyền ngược về mạch nguồn gây treo chip ESP32.

### 4.3. Thiết kế Phủ đồng Tiếp địa (GND Plane) hiệu quả
*   **GND Lớp Bottom:** Phủ đồng toàn bộ lớp Bottom với net mạng `GND`. Điều này tạo ra một "mặt chắn đất" triệt tiêu sóng điện từ trường từ môi trường xung quanh.
*   **Ngăn cách rãnh đất (Ground Split):** 
    *   Tuyệt đối **không phủ đồng GND** xuyên qua rãnh cách ly của khu vực 220V AC.
    *   Đường GND của phần nguồn LM2596 (dòng điện lớn, nhiễu tần số cao) nên được kết nối về chân GND nguồn chính qua một điểm duy nhất (Single-point grounding) để tránh dòng nhiễu tần số cao chạy vòng vèo qua chân GND của ESP32.
*   **Via tiếp địa (Via Stitching):** Sử dụng nhiều lỗ via nối đất liên kết đồng GND ở cả lớp Top và Bottom để giảm trở kháng của đường đất.

---

## 5. Danh sách các quy tắc kiểm tra nhanh trên Altium/KiCad (Rule Checklist)

1.  [ ] **Clearance Constraint (Low voltage):** $\ge 10\text{ mil}$ ($0.254\text{ mm}$).
2.  [ ] **Clearance Constraint (High voltage AC):** $\ge 100\text{ mil}$ ($2.54\text{ mm}$) giữa dây nóng, nguội và dây tín hiệu DC.
3.  [ ] **Track Width Constraint:** Thiết lập các nhóm mạng (Net Class) như `POWER_12V` ($80\text{mil}$), `POWER_5V_3V3` ($30\text{mil}$), và `SIGNAL` ($10\text{mil}$).
4.  [ ] **Board Cutout:** Đã vẽ rãnh rộng $3\text{ mm}$ cô lập dưới các Relay.
5.  [ ] **Decoupling Caps:** Các tụ lọc $100\text{ nF}$ đã nằm sát chân chip và đầu cắm cảm biến chưa?
6.  [ ] **ESP32 Antenna:** Không có bất kỳ đường dây đồng hay vùng phủ đồng nào nằm dưới anten module ESP32-S3 ở cả hai lớp.
7.  [ ] **Thermal Relief:** Các chân GND nối vào lớp phủ đồng đất nên sử dụng kiểu kết nối tản nhiệt (Thermal Relief) để dễ hàn thiếc bằng tay, không bị tản nhiệt vào bo mạch lớn.
