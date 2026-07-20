# CẤU TRÚC THƯ MỤC DỰ ÁN — PHIÊN BẢN CHÍNH THỨC
## Đồ Án: Ứng Dụng AIoT Digital Twin Trồng Xà Lách Thủy Canh NFT

> Cập nhật sau khi đọc toàn bộ KE_HOACH_DO_AN.md + toàn bộ code hiện có

---

## TỔNG QUAN HỆ THỐNG (Từ plan)

```
ESP32-S3 (Firmware)
   ├── Đọc 6 loại cảm biến (SHT31, DS18B20, TDS, pH, BH1750, Y26×4)
   ├── Điều khiển 8 kênh PWM MOSFET (3 bơm DD + 1 bơm chìm + 4 quạt)
   ├── Điều khiển 6 kênh Relay (1 bơm 220V + 1 sục 220V + 4 đèn)
   └── Gửi/nhận MQTT ↔ Raspberry Pi

Raspberry Pi 4 (Trung tâm xử lý)
   ├── MQTT Broker (Mosquitto)
   ├── InfluxDB (Lưu dữ liệu cảm biến theo thời gian)
   ├── FastAPI Backend (API + WebSocket)
   ├── CNN MobileNetV2 (TFLite) — Nhận diện bệnh lá từ ảnh chụp
   ├── XGBoost — Dự đoán ngày thu hoạch
   ├── OPC UA Server (asyncua)
   └── Web Dashboard (HTML/CSS/JS + Digital Twin 2D)

PC Laptop (Phát triển + Digital Twin 3D)
   ├── Siemens NX MCD ← kết nối OPC UA từ Pi
   ├── Train CNN trên Google Colab
   └── Train XGBoost trên PC
```

---

## CẤU TRÚC THƯ MỤC ĐỀ XUẤT

```
DATN_START/
│
│── 📂 docs/                              ← TÀI LIỆU & BÁO CÁO
│   ├── Tailieuthamkhao/                  ← (có sẵn) Bài báo, đồ án tham khảo
│   ├── Bao_Cao_Tien_Do/                 ← (có sẵn) Báo cáo tiến độ
│   ├── Bao_cao_He_thong_AIoT_Digital_Twin.pptx  ← (có sẵn)
│   ├── Bao_cao_tien_do_thuc_hien_KLTN.docx      ← (có sẵn)
│   ├── Sơ đồ khối hệ thống.pdf                  ← (có sẵn)
│   └── KE_HOACH_DO_AN.md               ← (có sẵn, từ plan_viewer)
│
│── 📂 hardware/                          ← PHẦN CỨNG
│   ├── PCB_Design/                      ← (có sẵn) Schematic, layout, Gerber
│   │   ├── Schematic_DATN-VER2.pdf
│   │   ├── PCB_CHECKLIST.md
│   │   ├── pcb_layout_guide.md
│   │   └── huong_dan_xuat_gerber.md
│   └── BanVeCK/                         ← (có sẵn) Bản vẽ cơ khí NX (.prt)
│       ├── LapRap.prt                   ← Assembly tổng
│       ├── KHO_A3.prt                   ← Bản vẽ kỹ thuật
│       ├── quattannhiet.prt             ← Quạt tản nhiệt
│       └── ... (các chi tiết khác)
│
│── 📂 firmware/                          ← CODE ESP32-S3 (Tối ưu hóa & Làm sạch)
│   ├── include/                          ← Thư mục Header C++
│   │   ├── config.h                      ← Cấu hình WiFi, MQTT, GPIO pins
│   │   ├── ota_handler.h                 ← Khai báo hàm OTA & Web Server
│   │   ├── thresholds.h                  ← Ngưỡng cảm biến (EC, pH, nhiệt độ)
│   │   └── web_assets.h                  ← File nén Web UI tự động sinh ra từ thư mục web/
│   ├── src/                              ← Thư mục Source C++
│   │   ├── main.cpp                      ← Điểm khởi đầu & Khởi tạo nhiệm vụ FreeRTOS
│   │   ├── sensors.cpp                   ← Driver đọc các cảm biến (I2C, OneWire, ADC)
│   │   ├── actuator.cpp                  ← Lớp điều khiển an toàn Relay & PWM MOSFET
│   │   ├── ota_handler.cpp               ← Web Server local, Captive Portal, WebSocket
│   │   ├── mqtt_handler.cpp              ← Lớp truyền thông MQTT kết nối Raspberry Pi 4
│   │   └── failsafe.cpp                  ← Logic bảo vệ & tự động khi offline
│   ├── web/                              ← GIAO DIỆN WEB LOCAL CỦA ESP32 (DEV PHẦN WEB RIÊNG)
│   │   ├── index.html                    ← Trang dashboard giám sát & cấu hình local
│   │   ├── style.css                     ← CSS định dạng giao diện hiện đại
│   │   └── script.js                     ← Logic JS kết nối WebSocket điều khiển & hiển thị
│   ├── platformio.ini                    ← Tệp cấu hình dự án PlatformIO
│   └── generate_web_assets.py            ← Script Python tự động nén web sang web_assets.h khi build
│
│── 📂 backend/                           ← FASTAPI CHẠY TRÊN RASPBERRY PI
│   ├── main.py                          ← Khởi tạo FastAPI app
│   ├── routers/
│   │   ├── sensors.py                   ← GET /api/sensor-data/latest & history
│   │   ├── actuators.py                ← POST /api/control/pwm & relay
│   │   ├── prediction.py              ← GET /api/ai/growth-predict (XGBoost)
│   │   ├── disease.py                 ← POST /api/ai/disease-detect (CNN)
│   │   └── ota.py                     ← API upload và kích hoạt nạp code OTA (MỚI)
│   ├── services/
│   │   ├── mqtt_client.py              ← Subscribe MQTT từ ESP32
│   │   ├── influxdb_client.py          ← Đọc/ghi InfluxDB
│   │   ├── opcua_server.py             ← OPC UA Server (asyncua)
│   │   ├── xgboost_service.py          ← Load .pkl + predict sinh trưởng
│   │   └── cnn_service.py              ← Load TFLite + predict bệnh lá
│   ├── models/
│   │   └── schemas.py                  ← Pydantic: SensorData, ControlCommand...
│   ├── static/                          ← Thư mục static phục vụ Web và OTA
│   │   └── firmware/                   ← Nơi lưu trữ file .bin biên dịch để ESP32 tải về (MỚI)
│   ├── requirements.txt
│   ├── Dockerfile
│   └── .env                            ← MQTT_HOST, INFLUX_URL, secrets
│
│── 📂 frontend/                          ← WEB DASHBOARD
│   ├── index.html                       ← Trang chính
│   ├── style.css                        ← Giao diện
│   ├── app.js                           ← Logic JS (WebSocket, Chart.js, Upload OTA)
│   ├── digital_twin_2d.js              ← Canvas/SVG Digital Twin 2D trên Web
│   ├── disease_upload.js               ← Chụp ảnh camera ĐT → upload CNN
│   └── assets/                         ← Hình ảnh, icon
│
│── 📂 ai_model/                          ← HUẤN LUYỆN AI (TRÊN PC/COLAB)
│   ├── xgboost/
│   │   ├── data/
│   │   │   ├── lettuce_dataset.csv     ← Dataset sinh trưởng
│   │   │   └── preprocessing.py        ← Tiền xử lý dữ liệu
│   │   ├── train_xgboost.py            ← Huấn luyện model
│   │   ├── hyperparameter_tuning.py    ← GridSearch tối ưu
│   │   ├── evaluate.py                 ← Đánh giá MAE, RMSE, R²
│   │   └── saved_models/
│   │       └── lettuce_growth.pkl      ← Model → copy sang Pi backend
│   │
│   └── cnn/
│       ├── data/
│       │   └── leaf_disease_images/    ← Ảnh lá bệnh (train trên Colab)
│       ├── train_cnn_colab.ipynb       ← Notebook train MobileNetV2 trên Colab
│       ├── convert_tflite.py           ← Chuyển model → TFLite cho Pi
│       └── saved_models/
│           └── disease_mobilenetv2.tflite  ← Model → copy sang Pi backend
│
│── 📂 digital_twin/                      ← DỰ ÁN 3D DIGITAL TWIN TRÊN PC (SIEMENS NX)
│   ├── NX_MCD_Project/                 ← Thư mục chứa file lắp ráp và cấu hình động học 3D (.prt)
│   └── nx_mcd_real_connection.md       ← Hướng dẫn ánh xạ (map) tín hiệu thật từ OPC UA Server của Pi sang NX MCD
│
│── 📂 learning/                          ← TỰ HỌC (KHÔNG DEPLOY LÊN PI)
│   ├── Hoc_FastAPI/                    ← (có sẵn) Bài học FastAPI
│   ├── XGBOOST/                        ← (có sẵn) Bài học XGBoost
│   └── Cau_Truc_Du_An/                ← Thư mục này
│
│── 📂 tools/                             ← CÔNG CỤ PHỤ TRỢ
│   ├── diagram_viewer/                 ← (có sẵn) Web xem sơ đồ hệ thống
│   ├── plan_viewer/                    ← (có sẵn) Web xem kế hoạch
│   └── photo/                          ← (có sẵn) Ảnh tham khảo
│
├── docker-compose.yml                   ← Khởi chạy hệ thống trên Pi
└── DATN_START.code-workspace            ← (có sẵn) VS Code workspace
```

---

## GIẢI THÍCH TỪNG THƯ MỤC

### `docs/` — Tài liệu không chạy
Gom **tất cả** file báo cáo, PowerPoint, PDF, tài liệu tham khảo vào đây. Khi hội đồng bảo vệ yêu cầu xem tài liệu, bạn chỉ cần mở 1 thư mục duy nhất.

### `hardware/` — Phần cứng
Gom PCB + bản vẽ cơ khí. Hai thứ này liên quan chặt (bản vẽ cơ khí quyết định vị trí đặt PCB, quạt, cảm biến).

### `firmware/` — Code ESP32 (Tối ưu hóa & Làm sạch)
Thư mục chứa mã nguồn C++ (PlatformIO) và giao diện Web cấu hình local trực tiếp trên ESP32:
- **web/**: Chứa các tệp giao diện Web nguồn (`index.html`, `style.css`, `script.js`) phục vụ phát triển, lập trình và kiểm thử giao diện điều khiển local trước khi kết nối Pi.
- **src/ & include/**: Chứa mã nguồn C++ đọc cảm biến, quản lý rơ-le, FreeRTOS và script `generate_web_assets.py` giúp nén toàn bộ thư mục `web/` thành byte-code trong `include/web_assets.h`.

### `backend/` — Trái tim của hệ thống
Đây là thư mục **quan trọng nhất**, chạy trên Raspberry Pi, bao gồm:
- FastAPI (API endpoints cho Web)
- MQTT Client (nhận data từ ESP32)
- InfluxDB Client (lưu trữ dữ liệu chuỗi thời gian)
- OPC UA Server (cầu nối Digital Twin 3D)
- XGBoost Service (dự đoán sinh trưởng)
- **CNN Service** (nhận diện bệnh lá — đây là phần bạn bỏ sót!)

### `frontend/` — Giao diện Web
Dashboard hiển thị real-time + Digital Twin 2D + tính năng chụp ảnh bệnh lá.

### `ai_model/` — Huấn luyện AI
Chia rõ 2 mô hình:
- **xgboost/** — Train trên PC, xuất `.pkl`
- **cnn/** — Train trên Google Colab (vì cần GPU), xuất `.tflite` cho Pi

### `digital_twin/` — Dự án 3D Digital Twin trên PC (Siemens NX)
Chứa các file dự án 3D chính thức của mô hình giàn cây và hướng dẫn đồng bộ:
* **`NX_MCD_Project/`**: Nơi lưu các file vẽ 3D lắp ráp hoàn thiện (.prt) và các file cấu hình khớp động học, bộ điều khiển tốc độ trong Siemens NX MCD.
* **`nx_mcd_real_connection.md`**: Hướng dẫn kết nối trực tiếp Siemens NX MCD từ Laptop tới địa chỉ IP mạng LAN của Raspberry Pi 4 / VM Ubuntu qua cổng `4840` để đồng bộ chuyển động 3D theo đúng dữ liệu thực tế từ cảm biến và lệnh điều khiển của ESP32.
* **Lưu ý:** Các file chạy thử giả lập offline (`opcua_simulator.py`, `test_fans.py`...) đã được chuyển sang thư mục thử nghiệm độc lập (`Test_Linh_Tinh/`) để giữ cho thư mục Digital Twin này sạch sẽ, chuyên nghiệp.

### `learning/` — Tự học
Tất cả file hướng dẫn, bài tập thực hành. **Không deploy** lên Pi.

### `tools/` — Công cụ phụ trợ
Các web viewer nhỏ bạn đã tạo (diagram_viewer, plan_viewer).

---

## DI CHUYỂN FILE TỪ CẤU TRÚC HIỆN TẠI

| File/Thư mục hiện tại | Di chuyển sang |
|---|---|
| `Tailieuthamkhao/` | → `docs/Tailieuthamkhao/` |
| `Bao_Cao_Tien_Do/` | → `docs/Bao_Cao_Tien_Do/` |
| `Bao_cao_*.pptx`, `Bao_cao_*.docx` | → `docs/` |
| `Sơ đồ khối hệ thống.pdf` | → `docs/` |
| `PCB_Design/` | → `hardware/PCB_Design/` |
| `BanVeCK/` | → `hardware/BanVeCK/` |
| `opcua_simulator.py` | → `digital_twin/` |
| `test_fans.py` | → `digital_twin/` |
| `nx_mcd_*.md` (2 file) | → `digital_twin/` |
| `diagram_viewer/` | → `tools/diagram_viewer/` |
| `plan_viewer/` | → `tools/plan_viewer/` |
| `photo/` | → `tools/photo/` |
| `Hoc_FastAPI/` | → `learning/Hoc_FastAPI/` |
| `XGBOOST/` | → `learning/XGBOOST/` |
| `Cau_Truc_Du_An/` | → `learning/Cau_Truc_Du_An/` |
| `__pycache__/` | Xóa (tự sinh, không cần giữ) |

> **Lưu ý:** Bạn có thể nhờ tôi chạy lệnh di chuyển tự động khi bạn đã sẵn sàng.

---

## CÁI GÌ CHẠY Ở ĐÂU?

```
┌──────────────────────┐     WiFi/MQTT      ┌──────────────────────────────┐
│      ESP32-S3        │ ──────────────────► │       Raspberry Pi 4         │
│                      │                     │                              │
│  firmware/           │                     │  backend/    (FastAPI)       │
│  • Đọc 6 cảm biến   │                     │  • MQTT Client (nhận data)   │
│  • PWM 8 kênh        │                     │  • InfluxDB (lưu trữ)       │
│  • Relay 6 kênh      │                     │  • XGBoost (sinh trưởng)     │
│  • Failsafe tự động  │                     │  • CNN TFLite (bệnh lá)     │
│                      │ ◄────────────────── │  • OPC UA Server             │
│  Nhận lệnh PWM/Relay │     MQTT            │  • WebSocket (real-time)     │
└──────────────────────┘                     │                              │
                                             │  frontend/  (Web Dashboard) │
                                             │  • Chart.js biểu đồ         │
                                             │  • Digital Twin 2D           │
                                             │  • Upload ảnh bệnh lá       │
                                             └──────────┬───────────────────┘
                                                        │ OPC UA (TCP)
                                             ┌──────────┴───────────────────┐
                                             │       PC (Laptop)            │
                                             │                              │
                                             │  digital_twin/ (NX MCD 3D)  │
                                             │  ai_model/    (Train AI)    │
                                             │  learning/    (Tự học)      │
                                             └──────────────────────────────┘
```

---

## KHI NÀO DÙNG DOCKER?

| Giai đoạn | Dùng Docker? | Lý do |
|---|:---:|---|
| Đang học trên PC (hiện tại) | ❌ | Chạy trực tiếp `uvicorn`, `python` cho nhanh |
| Train XGBoost / CNN | ❌ | Chạy trực tiếp hoặc Google Colab |
| Lập trình ESP32 firmware | ❌ | Nạp code trực tiếp qua UART |
| NX MCD Digital Twin | ❌ | Phần mềm desktop |
| **Deploy Backend lên Pi** | ✅ | Đảm bảo môi trường giống PC |
| **MQTT Broker trên Pi** | ✅ | 1 lệnh Docker, không cần cài đặt phức tạp |
| **InfluxDB trên Pi** | ✅ | Database cần cách ly, tránh xung đột |
| **Nginx (serve frontend)** | ✅ | Phục vụ HTML/CSS/JS ổn định |

**docker-compose.yml trên Pi:**
```yaml
version: "3.8"
services:
  mosquitto:
    image: eclipse-mosquitto:2
    ports: ["1883:1883"]
    restart: always

  influxdb:
    image: influxdb:2
    ports: ["8086:8086"]
    volumes: ["influx_data:/var/lib/influxdb2"]
    restart: always

  backend:
    build: ./backend
    ports: ["8000:8000"]
    depends_on: [mosquitto, influxdb]
    restart: always

  frontend:
    image: nginx:alpine
    ports: ["80:80"]
    volumes: ["./frontend:/usr/share/nginx/html:ro"]
    restart: always

volumes:
  influx_data:
```

Chạy tất cả trên Pi bằng 1 lệnh: `docker-compose up -d`
