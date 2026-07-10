# HƯỚNG DẪN HỌC FASTAPI CHO ĐỒ ÁN DIGITAL TWIN THỦY CANH NFT

## Bạn Cần FastAPI Để Làm Gì Trong Đồ Án?

Trong kiến trúc hệ thống của bạn, **FastAPI** đóng vai trò **Backend trung tâm**:

```
[Web Dashboard] ←──WebSocket/API──→ [FastAPI Backend] ←──Đọc/Ghi──→ [OPC UA Server]
                                          ↕
                                    [XGBoost Model]
```

**Cụ thể FastAPI sẽ làm những việc sau:**
1. Cung cấp **REST API** để Web Dashboard đọc dữ liệu cảm biến (nhiệt độ, pH, TDS...)
2. Cung cấp API để Web Dashboard **điều khiển thiết bị** (bật/tắt quạt, bơm, đèn)
3. Gọi **XGBoost model** dự đoán sinh trưởng xà lách và trả kết quả về Web
4. Truyền dữ liệu **real-time** qua **WebSocket** để Dashboard cập nhật liên tục
5. Kết nối với **OPC UA Server** (`opcua_simulator.py`) để đọc/ghi tín hiệu

---

## PHẦN 1: KIẾN THỨC PYTHON CẦN BIẾT TRƯỚC

### 1.1 async/await (Lập trình bất đồng bộ) ⭐ QUAN TRỌNG NHẤT

Bạn đã dùng `async/await` trong `opcua_simulator.py` rồi. FastAPI cũng dùng cơ chế này.

**Tại sao cần?** Khi server nhận 100 request cùng lúc, nếu không có async, request thứ 2 phải đợi request thứ 1 xong mới chạy. Với async, tất cả chạy đan xen nhau.

```python
# ĐỒNG BỘ (chậm) - Request 2 phải chờ Request 1 xong
def doc_cam_bien():
    data = doc_tu_opcua()      # Đợi 0.5 giây
    return data

# BẤT ĐỒNG BỘ (nhanh) - Trong lúc đợi OPC UA, xử lý request khác
async def doc_cam_bien():
    data = await doc_tu_opcua()  # Đợi 0.5 giây, nhưng CPU đi làm việc khác
    return data
```

### 1.2 Dictionary và JSON

FastAPI tự động chuyển đổi giữa **Python dict** ↔ **JSON**. Bạn cần hiểu rõ:

```python
# Python dict
sensor_data = {"temperature": 27.5, "ph": 6.1, "tds": 750}

# FastAPI tự động chuyển thành JSON gửi cho Web App:
# {"temperature": 27.5, "ph": 6.1, "tds": 750}
```

### 1.3 Type Hints (Gợi ý kiểu dữ liệu)

FastAPI dựa rất nhiều vào type hints để tự động validate dữ liệu:

```python
# Không có type hint
def tinh(a, b):
    return a + b

# Có type hint - FastAPI sẽ tự kiểm tra kiểu dữ liệu đầu vào
def tinh(a: int, b: int) -> int:
    return a + b
```

### 1.4 Decorator (@) — Bạn đã hỏi và hiểu rồi ✅

---

## PHẦN 2: HỌC FASTAPI TỪ CƠ BẢN

### 2.1 Cài đặt

```bash
pip install fastapi uvicorn
```
- `fastapi`: Thư viện web framework
- `uvicorn`: Server để chạy ứng dụng FastAPI

### 2.2 Chương trình đầu tiên (Hello World)

Tạo file `main.py`:

```python
from fastapi import FastAPI

app = FastAPI()

@app.get("/")
async def trang_chu():
    return {"message": "Xin chào! Đây là hệ thống thủy canh NFT"}
```

Chạy bằng lệnh:
```bash
uvicorn main:app --reload --port 8000
```

Giải thích từng phần:
- `main`: tên file `main.py`
- `app`: tên biến `app = FastAPI()` trong file
- `--reload`: tự khởi động lại khi bạn sửa code
- `--port 8000`: chạy trên cổng 8000

Mở trình duyệt truy cập: `http://127.0.0.1:8000` → Thấy dòng JSON hiện ra.

### 2.3 Trang tài liệu API tự động (Swagger UI)

FastAPI **tự động tạo trang tài liệu** cho tất cả API bạn viết:
- `http://127.0.0.1:8000/docs` → Giao diện Swagger (có nút bấm thử API)
- `http://127.0.0.1:8000/redoc` → Giao diện ReDoc (đẹp hơn, dạng tài liệu)

Đây là điểm mạnh lớn nhất của FastAPI so với Flask. Bạn không cần viết tài liệu API riêng.

---

## PHẦN 3: CÁC KIẾN THỨC FASTAPI CẦN CHO ĐỒ ÁN

### 3.1 HTTP Methods (Phương thức HTTP) — Cần biết 3 loại

| Phương thức | Ý nghĩa | Ví dụ trong đồ án |
|------------|---------|-------------------|
| **GET** | Đọc/lấy dữ liệu | Đọc nhiệt độ, pH, TDS từ cảm biến |
| **POST** | Gửi dữ liệu lên để xử lý | Gửi dữ liệu cảm biến cho XGBoost dự đoán |
| **PUT** | Cập nhật/thay đổi trạng thái | Bật/tắt quạt, bơm, đèn |

```python
# GET - Đọc dữ liệu cảm biến
@app.get("/api/sensors")
async def doc_cam_bien():
    return {"temperature": 27.5, "ph": 6.1}

# POST - Gửi dữ liệu để dự đoán
@app.post("/api/predict-growth")
async def du_doan(sensor_data: dict):
    ket_qua = model.predict(...)
    return {"growth_percent": ket_qua}

# PUT - Điều khiển thiết bị
@app.put("/api/actuators/fan/{fan_id}")
async def dieu_khien_quat(fan_id: int, speed: int):
    # Ghi giá trị speed lên OPC UA
    return {"fan_id": fan_id, "speed": speed}
```

### 3.2 Path Parameters (Tham số đường dẫn)

Dùng để chỉ định **đối tượng cụ thể** trong URL:

```python
# {fan_id} là path parameter
@app.get("/api/fan/{fan_id}")
async def lay_toc_do_quat(fan_id: int):
    # fan_id = 1, 2, 3, hoặc 4
    return {"fan_id": fan_id, "speed": 255}
```

Khi Web App gọi `GET /api/fan/3` → FastAPI tự gán `fan_id = 3`.

### 3.3 Request Body (Dữ liệu gửi kèm) — Dùng Pydantic

Thay vì nhận `dict` chung chung, bạn nên dùng **Pydantic model** để FastAPI tự kiểm tra dữ liệu:

```python
from pydantic import BaseModel

# Định nghĩa cấu trúc dữ liệu đầu vào
class SensorData(BaseModel):
    temperature: float
    humidity: float
    water_temp: float
    ph: float
    tds: float
    light_intensity: float
    light_hours: float = 14.0       # Giá trị mặc định nếu không gửi
    day_of_growth: int = 1

# Sử dụng trong API
@app.post("/api/predict-growth")
async def du_doan(data: SensorData):
    # FastAPI tự động:
    # 1. Kiểm tra đủ các trường chưa
    # 2. Kiểm tra đúng kiểu (float, int) chưa
    # 3. Trả lỗi 422 nếu dữ liệu sai

    # Truy cập bằng data.temperature thay vì data["temperature"]
    return {"temperature_received": data.temperature}
```

**Lợi ích so với dùng `dict`:**
- Tự kiểm tra dữ liệu sai → trả lỗi rõ ràng
- Tự tạo tài liệu API đẹp trên Swagger
- Gợi ý code (autocomplete) trong IDE

### 3.4 Kết nối OPC UA từ FastAPI ⭐ ĐẶC THÙ ĐỒ ÁN

Đây là phần quan trọng nhất — kết nối FastAPI với `opcua_simulator.py`:

```python
from fastapi import FastAPI
from asyncua import Client

app = FastAPI()
OPCUA_URL = "opc.tcp://127.0.0.1:4840"
NAMESPACE_IDX = 2

async def doc_gia_tri_opcua(tag: str):
    """Đọc 1 giá trị từ OPC UA Server"""
    async with Client(url=OPCUA_URL) as client:
        node = client.get_node(f"ns={NAMESPACE_IDX};s={tag}")
        value = await node.read_value()
        return value

async def ghi_gia_tri_opcua(tag: str, value):
    """Ghi 1 giá trị lên OPC UA Server"""
    async with Client(url=OPCUA_URL) as client:
        node = client.get_node(f"ns={NAMESPACE_IDX};s={tag}")
        await node.write_value(float(value))

# API đọc tất cả cảm biến
@app.get("/api/sensors")
async def doc_tat_ca_cam_bien():
    return {
        "temperature": await doc_gia_tri_opcua("sensor.temp"),
        "humidity": await doc_gia_tri_opcua("sensor.humidity"),
        "ph": await doc_gia_tri_opcua("sensor.ph"),
        "tds": await doc_gia_tri_opcua("sensor.tds"),
        "water_temp": await doc_gia_tri_opcua("sensor.water_temp"),
        "light": await doc_gia_tri_opcua("sensor.light"),
    }

# API điều khiển quạt
@app.put("/api/fan/{fan_id}")
async def dieu_khien_quat(fan_id: int, speed: float):
    tag = f"actuator.fan_{fan_id}"
    await ghi_gia_tri_opcua(tag, speed)
    return {"fan_id": fan_id, "speed": speed, "status": "OK"}
```

### 3.5 WebSocket (Truyền dữ liệu real-time) ⭐ ĐẶC THÙ ĐỒ ÁN

REST API (GET/POST) hoạt động theo kiểu "hỏi-đáp": Web App hỏi → Server trả lời → xong.
Nhưng Dashboard cần cập nhật liên tục mỗi giây mà không cần người dùng refresh trang.

**WebSocket** giải quyết vấn đề này: mở kết nối 1 lần, sau đó server chủ động đẩy dữ liệu mới về liên tục.

```python
from fastapi import FastAPI, WebSocket
import asyncio
import json

app = FastAPI()

@app.websocket("/ws/sensors")
async def websocket_cam_bien(websocket: WebSocket):
    await websocket.accept()  # Chấp nhận kết nối WebSocket

    try:
        while True:
            # Đọc cảm biến từ OPC UA
            data = {
                "temperature": await doc_gia_tri_opcua("sensor.temp"),
                "ph": await doc_gia_tri_opcua("sensor.ph"),
                "tds": await doc_gia_tri_opcua("sensor.tds"),
            }
            # Gửi dữ liệu về cho Web App
            await websocket.send_json(data)
            await asyncio.sleep(1)  # Gửi mỗi giây

    except Exception:
        print("Client ngắt kết nối WebSocket")
```

**Phía Web App (JavaScript):**
```javascript
const ws = new WebSocket("ws://127.0.0.1:8000/ws/sensors");
ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    // Cập nhật giao diện Dashboard
    document.getElementById("temp").innerText = data.temperature + "°C";
    document.getElementById("ph").innerText = data.ph;
};
```

### 3.6 CORS (Cross-Origin Resource Sharing)

Khi Web App chạy ở `localhost:3000` mà FastAPI chạy ở `localhost:8000`, trình duyệt sẽ chặn kết nối (do bảo mật). Bạn cần thêm CORS middleware:

```python
from fastapi.middleware.cors import CORSMiddleware

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Cho phép tất cả (dev). Production nên giới hạn.
    allow_methods=["*"],
    allow_headers=["*"],
)
```

### 3.7 Static Files (Phục vụ file HTML/CSS/JS)

Nếu bạn muốn FastAPI phục vụ luôn Web Dashboard (không cần server riêng):

```python
from fastapi.staticfiles import StaticFiles

# Phục vụ thư mục "static" chứa HTML, CSS, JS
app.mount("/", StaticFiles(directory="static", html=True), name="static")
```

---

## PHẦN 4: CẤU TRÚC FILE BACKEND ĐỀ XUẤT CHO ĐỒ ÁN

```
DATN_START/
├── backend/
│   ├── main.py              ← File chính khởi động FastAPI
│   ├── routers/
│   │   ├── sensors.py       ← API đọc cảm biến (GET /api/sensors)
│   │   ├── actuators.py     ← API điều khiển thiết bị (PUT /api/fan/...)
│   │   └── prediction.py    ← API dự đoán XGBoost (POST /api/predict)
│   ├── services/
│   │   ├── opcua_client.py  ← Hàm đọc/ghi OPC UA (tái sử dụng)
│   │   └── xgboost_service.py ← Load model + predict
│   ├── models/
│   │   └── schemas.py       ← Pydantic models (SensorData, FanCommand...)
│   └── static/              ← HTML/CSS/JS cho Web Dashboard
│       ├── index.html
│       ├── style.css
│       └── app.js
├── opcua_simulator.py
├── XGBOOST/
└── ...
```

---

## PHẦN 5: CHECKLIST KIẾN THỨC CẦN HỌC

### Mức 1: Cơ bản (Bắt buộc) — 3-4 ngày
- [ ] Cài đặt FastAPI + uvicorn
- [ ] Tạo GET API đầu tiên (Hello World)
- [ ] Hiểu Swagger UI tự động (`/docs`)
- [ ] Path parameters (`/fan/{fan_id}`)
- [ ] Request Body với Pydantic BaseModel
- [ ] Trả về JSON response
- [ ] Chạy server bằng `uvicorn main:app --reload`

### Mức 2: Áp dụng đồ án (Cần thiết) — 3-4 ngày
- [ ] Kết nối FastAPI ↔ OPC UA Server (đọc/ghi cảm biến)
- [ ] Tạo API đọc tất cả cảm biến (`GET /api/sensors`)
- [ ] Tạo API điều khiển quạt/bơm/đèn (`PUT /api/actuators/...`)
- [ ] Tạo API dự đoán XGBoost (`POST /api/predict-growth`)
- [ ] Thêm CORS middleware
- [ ] WebSocket cho real-time data

### Mức 3: Nâng cao (Làm đẹp đồ án) — 2-3 ngày
- [ ] Tổ chức code theo Router (tách file)
- [ ] Xử lý lỗi (HTTPException, try/except)
- [ ] Phục vụ Static Files (Web Dashboard)
- [ ] Logging (ghi nhật ký hoạt động)
- [ ] Background Tasks (chạy tác vụ ngầm)

---

## PHẦN 6: TÀI LIỆU THAM KHẢO

### Tài liệu chính thức
1. [FastAPI Official Tutorial](https://fastapi.tiangolo.com/tutorial/) — Rất dễ hiểu, có tiếng Việt
2. [Pydantic Documentation](https://docs.pydantic.dev/)
3. [Uvicorn Documentation](https://www.uvicorn.org/)

### Video YouTube khuyên dùng
1. "FastAPI Full Course for Beginners" — freeCodeCamp (tiếng Anh, 4 giờ)
2. "FastAPI Tutorial" — Tech With Tim (ngắn gọn, dễ hiểu)
3. "Python API Development" — Sanjeev Thiyagarajan (19 giờ, rất chi tiết)

### Gợi ý thứ tự học
1. Xem video ngắn 30 phút để nắm tổng quan
2. Đọc FastAPI Official Tutorial phần First Steps
3. Code theo từng mục trong PHẦN 3 của file này
4. Chạy thử kết hợp với `opcua_simulator.py` đã có sẵn

> **MẸO:** Bạn đã viết được `opcua_simulator.py` và `test_fans.py` dùng async/await rồi — đó chính là nền tảng quan trọng nhất. FastAPI chỉ là thêm lớp HTTP bọc bên ngoài các hàm async đó mà thôi!
