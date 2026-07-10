# HƯỚNG DẪN HỌC XGBOOST & TÍCH HỢP NX MCD DIGITAL TWIN
## Dành cho Đồ Án: Ứng Dụng AIoT Trồng Xà Lách Thủy Canh NFT

---

## PHẦN 1: NỀN TẢNG CẦN HỌC TRƯỚC

### 1.1 Python Cơ Bản (1-2 ngày)
- Biến, vòng lặp, hàm, class
- Thư viện: `numpy`, `pandas`, `matplotlib`

### 1.2 Machine Learning Cơ Bản (2-3 ngày)
- **Supervised Learning**: Regression vs Classification
- **Train/Test Split**: Tách dữ liệu huấn luyện và kiểm tra
- **Overfitting vs Underfitting**: Hiểu khái niệm
- **Metrics đánh giá**: MAE, RMSE, R² Score

### 1.3 Decision Tree & Ensemble Methods (1-2 ngày)
- **Decision Tree**: Cây quyết định đơn lẻ
- **Bagging**: Random Forest (nhiều cây độc lập, bỏ phiếu)
- **Boosting**: Các cây học tuần tự, sửa lỗi cây trước
  - AdaBoost → Gradient Boosting → **XGBoost**

---

## PHẦN 2: HỌC XGBOOST CHI TIẾT

### 2.1 XGBoost Là Gì?
**XGBoost = eXtreme Gradient Boosting**

Là thuật toán Gradient Boosted Decision Trees được tối ưu hóa về tốc độ và hiệu năng. Nguyên lý hoạt động:

```
Cây 1 dự đoán → Sai số (Residual) → Cây 2 học từ sai số → Sai số mới → Cây 3 học tiếp → ...
Kết quả cuối = Tổng dự đoán của tất cả các cây
```

### 2.2 Tại Sao Chọn XGBoost Cho Đồ Án?
| Tiêu chí | XGBoost | Linear SVR | Random Forest |
|----------|---------|------------|---------------|
| Xử lý phi tuyến | ✅ Rất tốt | ❌ Yếu (kernel linear) | ✅ Tốt |
| Tốc độ huấn luyện | ✅ Nhanh | ✅ Nhanh | ⚠️ Trung bình |
| Chống overfitting | ✅ Regularization L1/L2 | ⚠️ Hạn chế | ⚠️ Trung bình |
| Feature Importance | ✅ Có sẵn | ❌ Không trực quan | ✅ Có |
| Dữ liệu nhỏ (<1000 mẫu) | ✅ Hoạt động tốt | ✅ Tốt | ⚠️ Cần nhiều dữ liệu |

### 2.3 Cài Đặt
```bash
pip install xgboost scikit-learn pandas numpy matplotlib
```

### 2.4 Các Hyperparameter Quan Trọng

| Tham số | Ý nghĩa | Giá trị khuyến nghị |
|---------|---------|---------------------|
| `n_estimators` | Số cây boosting | 100 - 500 |
| `max_depth` | Độ sâu tối đa mỗi cây | 3 - 8 |
| `learning_rate` (eta) | Tốc độ học, nhỏ = cẩn thận hơn | 0.01 - 0.3 |
| `subsample` | Tỉ lệ mẫu dùng mỗi cây | 0.7 - 1.0 |
| `colsample_bytree` | Tỉ lệ feature dùng mỗi cây | 0.7 - 1.0 |
| `reg_alpha` | Regularization L1 | 0 - 1 |
| `reg_lambda` | Regularization L2 | 1 - 5 |
| `objective` | Hàm mục tiêu | `reg:squarederror` (hồi quy) |

### 2.5 Code Mẫu: Dự Đoán Sinh Trưởng Xà Lách

```python
import pandas as pd
import numpy as np
import xgboost as xgb
from sklearn.model_selection import train_test_split, cross_val_score
from sklearn.metrics import mean_absolute_error, mean_squared_error, r2_score
import matplotlib.pyplot as plt
import joblib

# ============================================================
# BƯỚC 1: CHUẨN BỊ DỮ LIỆU
# ============================================================
# Giả lập dataset cảm biến thủy canh NFT
np.random.seed(42)
n_samples = 500

data = pd.DataFrame({
    'temperature': np.random.uniform(22, 35, n_samples),       # °C
    'humidity': np.random.uniform(50, 95, n_samples),           # %
    'water_temp': np.random.uniform(18, 30, n_samples),         # °C
    'ph': np.random.uniform(5.0, 7.5, n_samples),              # pH
    'tds': np.random.uniform(400, 1200, n_samples),             # ppm
    'light_intensity': np.random.uniform(5000, 25000, n_samples), # lux
    'light_hours': np.random.uniform(8, 18, n_samples),         # giờ/ngày
    'day_of_growth': np.random.randint(1, 35, n_samples),       # ngày
})

# Hàm tính growth_percent (mô phỏng quy luật sinh trưởng)
def calc_growth(row):
    base = row['day_of_growth'] / 35.0 * 100
    temp_factor = 1.0 - abs(row['temperature'] - 25) * 0.02
    ph_factor = 1.0 - abs(row['ph'] - 6.0) * 0.1
    tds_factor = 1.0 - abs(row['tds'] - 800) * 0.0003
    light_factor = min(row['light_hours'] / 14.0, 1.0)
    growth = base * temp_factor * ph_factor * tds_factor * light_factor
    return np.clip(growth + np.random.normal(0, 2), 0, 100)

data['growth_percent'] = data.apply(calc_growth, axis=1)

# ============================================================
# BƯỚC 2: CHIA DỮ LIỆU
# ============================================================
FEATURES = ['temperature', 'humidity', 'water_temp', 'ph',
            'tds', 'light_intensity', 'light_hours', 'day_of_growth']
TARGET = 'growth_percent'

X = data[FEATURES]
y = data[TARGET]

X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)

print(f"Training: {X_train.shape[0]} mẫu | Testing: {X_test.shape[0]} mẫu")

# ============================================================
# BƯỚC 3: HUẤN LUYỆN MÔ HÌNH XGBOOST
# ============================================================
model = xgb.XGBRegressor(
    n_estimators=200,
    max_depth=5,
    learning_rate=0.1,
    subsample=0.8,
    colsample_bytree=0.8,
    reg_alpha=0.1,
    reg_lambda=1.0,
    objective='reg:squarederror',
    random_state=42,
    verbosity=1
)

model.fit(
    X_train, y_train,
    eval_set=[(X_train, y_train), (X_test, y_test)],
    verbose=20
)

# ============================================================
# BƯỚC 4: ĐÁNH GIÁ MÔ HÌNH
# ============================================================
y_pred = model.predict(X_test)
mae = mean_absolute_error(y_test, y_pred)
rmse = np.sqrt(mean_squared_error(y_test, y_pred))
r2 = r2_score(y_test, y_pred)

print(f"\n=== KẾT QUẢ ĐÁNH GIÁ ===")
print(f"MAE  (Sai số tuyệt đối TB): {mae:.2f}%")
print(f"RMSE (Sai số bình phương TB): {rmse:.2f}%")
print(f"R²   (Hệ số xác định):       {r2:.4f}")

# ============================================================
# BƯỚC 5: TRỰC QUAN HÓA
# ============================================================
# 5a. Feature Importance
fig, axes = plt.subplots(1, 2, figsize=(14, 5))

xgb.plot_importance(model, ax=axes[0], max_num_features=8)
axes[0].set_title('Feature Importance (XGBoost)')

# 5b. Actual vs Predicted
axes[1].scatter(y_test, y_pred, alpha=0.5, s=20, color='teal')
axes[1].plot([0, 100], [0, 100], 'r--', linewidth=2)
axes[1].set_xlabel('Actual Growth %')
axes[1].set_ylabel('Predicted Growth %')
axes[1].set_title(f'Actual vs Predicted (R²={r2:.4f})')

plt.tight_layout()
plt.savefig('xgboost_results.png', dpi=150)
plt.show()

# ============================================================
# BƯỚC 6: LƯU MÔ HÌNH
# ============================================================
model.save_model('lettuce_growth_xgboost.json')
joblib.dump(model, 'lettuce_growth_xgboost.pkl')
print("Đã lưu mô hình thành công!")
```

### 2.6 Tối Ưu Hyperparameter (GridSearch / RandomSearch)

```python
from sklearn.model_selection import GridSearchCV

param_grid = {
    'n_estimators': [100, 200, 300],
    'max_depth': [3, 5, 7],
    'learning_rate': [0.01, 0.05, 0.1],
    'subsample': [0.7, 0.8, 1.0],
}

grid_search = GridSearchCV(
    estimator=xgb.XGBRegressor(objective='reg:squarederror', random_state=42),
    param_grid=param_grid,
    cv=5,                    # 5-Fold Cross Validation
    scoring='r2',
    n_jobs=-1,               # Dùng tất cả CPU
    verbose=1
)

grid_search.fit(X_train, y_train)

print(f"Best params: {grid_search.best_params_}")
print(f"Best R² score: {grid_search.best_score_:.4f}")

best_model = grid_search.best_estimator_
```

---

## PHẦN 3: TÍCH HỢP XGBOOST VÀO HỆ THỐNG DIGITAL TWIN

### 3.1 Kiến Trúc Tổng Thể

```
┌─────────────────────────────────────────────────────────────────┐
│                    MÁY TÍNH CỦA BẠN (PC)                       │
│                                                                 │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────────┐   │
│  │  Python OPC  │◄──►│  FastAPI      │◄──►│  Web Dashboard   │   │
│  │  UA Server   │    │  Backend     │    │  (React/HTML)    │   │
│  │  (port 4840) │    │  (port 8000) │    │                  │   │
│  └──────┬───────┘    └──────┬───────┘    └──────────────────┘   │
│         │                   │                                    │
│         │            ┌──────┴───────┐                            │
│         │            │  XGBoost     │                            │
│         │            │  Predictor   │                            │
│         │            │  Service     │                            │
│         │            └──────────────┘                            │
│         │                                                        │
│  ┌──────┴───────┐                                                │
│  │ Siemens NX   │  ← Đọc ai.growth_percent → Scale cây 3D      │
│  │ MCD 3D       │  ← Đọc ai.days_remaining → Hiển thị text     │
│  └──────────────┘                                                │
└─────────────────────────────────────────────────────────────────┘
```

### 3.2 Luồng Dữ Liệu Chi Tiết

```
1. Cảm biến ảo (Simulator) ghi giá trị lên OPC UA Server
   sensor.temp=27.5, sensor.ph=6.1, sensor.tds=750...

2. FastAPI Backend đọc dữ liệu cảm biến từ OPC UA Server

3. FastAPI gọi XGBoost model.predict() với dữ liệu cảm biến
   → Trả về: growth_percent = 45.2%, days_remaining = 16.5

4. FastAPI ghi kết quả dự đoán lên OPC UA Server
   ai.growth_percent = 45.2
   ai.days_remaining = 16.5

5. NX MCD đọc ai.growth_percent → Gán vào Scale Factor cây 3D
   → Cây ảo tự động lớn lên theo dự đoán AI

6. Web Dashboard hiển thị biểu đồ sinh trưởng real-time
```

### 3.3 Code: XGBoost Prediction Service

Tạo file `xgboost_predictor.py` trong thư mục XGBOOST:

```python
"""
XGBoost Prediction Service
Đọc cảm biến từ OPC UA → Dự đoán sinh trưởng → Ghi kết quả lên OPC UA
"""
import asyncio
import logging
import numpy as np
import joblib
from asyncua import Client

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("XGBoost_Predictor")

# Cấu hình
OPCUA_URL = "opc.tcp://127.0.0.1:4840"
MODEL_PATH = "lettuce_growth_xgboost.pkl"
NAMESPACE_URI = "http://hydroponics.digitaltwin"
PREDICT_INTERVAL = 5  # Dự đoán mỗi 5 giây

# Thứ tự features phải khớp với lúc huấn luyện
FEATURE_TAGS = [
    "sensor.temp",           # temperature
    "sensor.humidity",       # humidity
    "sensor.water_temp",     # water_temp
    "sensor.ph",             # ph
    "sensor.tds",            # tds
    "sensor.light",          # light_intensity
]

async def run_predictor():
    # Load mô hình XGBoost đã huấn luyện
    model = joblib.load(MODEL_PATH)
    logger.info(f"Đã load mô hình XGBoost từ: {MODEL_PATH}")

    async with Client(url=OPCUA_URL) as client:
        logger.info(f"Đã kết nối OPC UA Server: {OPCUA_URL}")

        # Lấy namespace index
        idx = await client.get_namespace_index(NAMESPACE_URI)

        # Lấy node references
        sensor_nodes = {}
        for tag in FEATURE_TAGS:
            node = client.get_node(f"ns={idx};s={tag}")
            sensor_nodes[tag] = node

        # Lấy node AI output
        node_growth = client.get_node(f"ns={idx};s=ai.growth_percent")
        node_days = client.get_node(f"ns={idx};s=ai.days_remaining")

        # Giả lập thêm 2 feature cố định (light_hours, day_of_growth)
        day_of_growth = 1

        while True:
            try:
                # 1. Đọc giá trị cảm biến từ OPC UA
                values = []
                for tag in FEATURE_TAGS:
                    val = await sensor_nodes[tag].read_value()
                    values.append(float(val))

                # Thêm light_hours và day_of_growth
                light_hours = 14.0  # Giả lập 14 giờ chiếu sáng/ngày
                values.append(light_hours)
                values.append(float(day_of_growth))

                # 2. Tạo input array cho XGBoost
                X_input = np.array([values])

                # 3. Dự đoán
                growth_percent = float(model.predict(X_input)[0])
                growth_percent = np.clip(growth_percent, 0, 100)
                days_remaining = max(0, 35 * (1.0 - growth_percent / 100.0))

                # 4. Ghi kết quả lên OPC UA Server
                await node_growth.write_value(round(growth_percent, 2))
                await node_days.write_value(round(days_remaining, 1))

                logger.info(
                    f"[AI] Input: Temp={values[0]:.1f}°C, pH={values[3]:.2f}, "
                    f"TDS={values[4]:.0f}ppm, Day={day_of_growth} | "
                    f"Predict: Growth={growth_percent:.2f}%, "
                    f"Days Left={days_remaining:.1f}"
                )

            except Exception as e:
                logger.error(f"Lỗi dự đoán: {e}")

            await asyncio.sleep(PREDICT_INTERVAL)

            # Tăng ngày (mô phỏng tăng tốc: +1 ngày mỗi 60 giây)
            day_of_growth = min(35, day_of_growth + (PREDICT_INTERVAL / 60))

if __name__ == "__main__":
    asyncio.run(run_predictor())
```

### 3.4 Tích Hợp Vào FastAPI Backend

```python
"""
Thêm endpoint dự đoán XGBoost vào FastAPI Backend
"""
from fastapi import FastAPI
import joblib
import numpy as np

app = FastAPI(title="Hydroponics Digital Twin API")

# Load model 1 lần khi khởi động server
model = joblib.load("XGBOOST/lettuce_growth_xgboost.pkl")

@app.post("/api/predict-growth")
async def predict_growth(sensor_data: dict):
    """
    Nhận dữ liệu cảm biến → Trả về dự đoán sinh trưởng
    Input: {"temperature": 27.5, "humidity": 75, "water_temp": 24,
            "ph": 6.1, "tds": 750, "light_intensity": 15000,
            "light_hours": 14, "day_of_growth": 10}
    """
    features = np.array([[
        sensor_data["temperature"],
        sensor_data["humidity"],
        sensor_data["water_temp"],
        sensor_data["ph"],
        sensor_data["tds"],
        sensor_data["light_intensity"],
        sensor_data["light_hours"],
        sensor_data["day_of_growth"],
    ]])

    growth_percent = float(model.predict(features)[0])
    growth_percent = np.clip(growth_percent, 0, 100)
    days_remaining = max(0, 35 * (1.0 - growth_percent / 100))

    return {
        "growth_percent": round(growth_percent, 2),
        "days_remaining": round(days_remaining, 1),
        "status": "harvesting" if growth_percent >= 90 else "growing"
    }
```

---

## PHẦN 4: TÍCH HỢP XGBOOST VÀO NX MCD

### 4.1 NX MCD Nhận Kết Quả AI Qua OPC UA

Khi XGBoost Predictor Service ghi `ai.growth_percent` và `ai.days_remaining` lên OPC UA Server, NX MCD sẽ đọc được các giá trị này.

**Cách cấu hình trong NX MCD:**

#### Bước 1: Tạo Signal nhận dữ liệu AI
1. Trong NX MCD, tạo 2 Signal mới:
   - `mcd_growth_percent` (Double, Input)
   - `mcd_days_remaining` (Double, Input)

#### Bước 2: Map OPC UA tag AI vào Signal
Trong External Signal Configuration:
- Map `ns=2;s=ai.growth_percent` → `mcd_growth_percent`
- Map `ns=2;s=ai.days_remaining` → `mcd_days_remaining`

#### Bước 3: Gán Scale Factor cho cây 3D
1. Chọn cụm mô hình cây xà lách 3D.
2. Tạo một `Position Control` hoặc sử dụng `Runtime Expression`:
   - Scale X = `0.3 + (mcd_growth_percent / 100.0) * 0.7`
   - Scale Y = `0.3 + (mcd_growth_percent / 100.0) * 0.7`
   - Scale Z = `0.3 + (mcd_growth_percent / 100.0) * 0.7`
   - Giải thích: Khi growth = 0% → cây ở kích thước 30%. Khi growth = 100% → cây đạt 100%.

#### Bước 4: Hiển thị Days Remaining bằng Dynamic Text
1. Tạo một 3D Text annotation trên mô hình.
2. Bind giá trị text vào signal `mcd_days_remaining`.
3. Format: `"Thu hoạch sau: {mcd_days_remaining} ngày"`

### 4.2 Kịch Bản Chạy Hoàn Chỉnh

Mở **4 Terminal** và chạy theo thứ tự:

```bash
# Terminal 1: OPC UA Server + Simulator
python opcua_simulator.py

# Terminal 2: XGBoost Predictor
python XGBOOST/xgboost_predictor.py

# Terminal 3: FastAPI Backend
uvicorn main:app --reload --port 8000

# Terminal 4: Mở Siemens NX MCD → Play Simulation
```

**Kết quả mong đợi:**
- Web Dashboard hiển thị % sinh trưởng và biểu đồ dự đoán real-time
- NX MCD: Cây xà lách 3D tự động lớn dần theo dự đoán XGBoost
- NX MCD: Text 3D hiển thị số ngày còn lại đến thu hoạch
- Quạt, bơm, đèn vẫn hoạt động đồng bộ như trước

---

## PHẦN 5: CẤU TRÚC THƯ MỤC ĐỀ XUẤT

```
DATN_START/
├── XGBOOST/
│   ├── HUONG_DAN_XGBOOST_DIGITAL_TWIN.md   ← File này
│   ├── data/
│   │   ├── lettuce_dataset.csv              ← Dataset thực tế
│   │   └── data_preprocessing.py            ← Tiền xử lý dữ liệu
│   ├── training/
│   │   ├── train_xgboost.py                 ← Huấn luyện mô hình
│   │   ├── hyperparameter_tuning.py         ← Tối ưu tham số
│   │   └── model_evaluation.py              ← Đánh giá & so sánh
│   ├── models/
│   │   ├── lettuce_growth_xgboost.json      ← Model đã train
│   │   └── lettuce_growth_xgboost.pkl       ← Model (joblib)
│   ├── xgboost_predictor.py                 ← Service dự đoán real-time
│   └── xgboost_results.png                  ← Biểu đồ kết quả
├── opcua_simulator.py                        ← OPC UA Server (đã có)
├── test_fans.py                              ← Test quạt (đã có)
└── ...
```

---

## PHẦN 6: TÀI LIỆU THAM KHẢO

### Tài liệu chính thức
1. [XGBoost Documentation](https://xgboost.readthedocs.io/)
2. [Scikit-learn User Guide](https://scikit-learn.org/stable/user_guide.html)
3. [OPC UA asyncua Python](https://github.com/FreeOpcUa/opcua-asyncio)

### Video học tập (YouTube)
1. "StatQuest: XGBoost" - Josh Starmer (giải thích trực quan, dễ hiểu)
2. "XGBoost in Python from Start to Finish" - Normalize Nerd
3. "Gradient Boosting clearly explained" - StatQuest

### Dataset xà lách gợi ý
1. [Kaggle: Lettuce Growth Prediction](https://www.kaggle.com/) - Tìm "lettuce growth"
2. Tự thu thập từ cảm biến thực tế (khuyến khích)
3. Tạo dataset giả lập bằng code mẫu ở Phần 2.5

---

## PHẦN 7: LỘ TRÌNH THỰC HIỆN (TIMELINE)

| Tuần | Nội dung | Output |
|------|---------|--------|
| 1 | Học Python ML cơ bản + Decision Tree | Notebook ghi chú |
| 2 | Học XGBoost lý thuyết + code mẫu | Script chạy được |
| 3 | Thu thập/tạo dataset xà lách | File CSV dataset |
| 4 | Huấn luyện + tối ưu XGBoost | Model .pkl + báo cáo |
| 5 | Viết Predictor Service + tích hợp OPC UA | Service chạy real-time |
| 6 | Tích hợp NX MCD + test toàn hệ thống | Demo Digital Twin hoàn chỉnh |

---

> **GHI CHÚ**: File này là roadmap tổng quan. Mỗi phần có thể mở rộng thêm chi tiết khi bạn thực hiện đến bước đó. Hãy bắt đầu từ Phần 2.5 (chạy code mẫu) để nắm cảm giác thực tế trước!
