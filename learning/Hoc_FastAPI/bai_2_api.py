from fastapi import FastAPI
from pydantic import BaseModel

app = FastAPI(title="Gia Su AI - Bai 2: GET & POST")

# ============================================================
# PHẦN 1: GET API VỚI THAM SỐ ĐƯỜNG DẪN (PATH PARAMETERS)
# ============================================================
# Giả sử ta muốn đọc trạng thái của 1 trong 4 quạt: quạt 1, 2, 3, hoặc 4
@app.get("/api/fan/{fan_id}")
async def get_fan_status(fan_id: int):
    """
    API đọc trạng thái quạt theo ID.
    Tham số fan_id truyền trực tiếp trên URL. Ví dụ: /api/fan/3
    FastAPI sẽ tự kiểm tra xem fan_id có phải là số nguyên (int) hay không.
    """
    # Trả về dữ liệu giả lập cho từng quạt
    return {
        "fan_id": fan_id,
        "status": "Running" if fan_id % 2 == 0 else "Stopped",
        "speed_pwm": 255 if fan_id % 2 == 0 else 0
    }

# ============================================================
# PHẦN 2: POST API VỚI PYDANTIC MODEL (DỮ LIỆU GỬI KÈM)
# ============================================================
# 1. Định nghĩa cấu trúc dữ liệu môi trường gửi lên bằng Pydantic
class EnvironmentData(BaseModel):
    temperature: float      # Bắt buộc là số thực
    humidity: float         # Bắt buộc là số thực
    ph: float = 6.0         # Không bắt buộc, nếu không gửi mặc định là 6.0

# 2. Tạo API POST để kiểm tra điều kiện môi trường có thích hợp trồng xà lách không
@app.post("/api/check-environment")
async def check_environment(env: EnvironmentData):
    """
    API nhận thông số môi trường qua Body (dạng JSON)
    và đánh giá xem môi trường có tốt không.
    """
    # Lấy các biến ra để xử lý logic
    temp = env.temperature
    hum = env.humidity
    ph_level = env.ph

    # Logic đánh giá đơn giản
    status = "OK"
    warnings = []

    if temp > 30.0:
        status = "Warning"
        warnings.append("Nhiệt độ quá cao (trên 30°C). Cần bật thêm quạt!")
    elif temp < 18.0:
        status = "Warning"
        warnings.append("Nhiệt độ quá thấp (dưới 18°C). Cây phát triển chậm!")

    if ph_level < 5.5 or ph_level > 6.5:
        status = "Warning"
        warnings.append("Độ pH nằm ngoài khoảng tối ưu (5.5 - 6.5) cho xà lách!")

    return {
        "status": status,
        "warnings": warnings,
        "received_data": {
            "temp": temp,
            "humidity": hum,
            "ph": ph_level
        }
    }
