from fastapi import FastAPI

# 1. Khởi tạo một ứng dụng FastAPI
app = FastAPI(title="Gia Su AI - Bai 1: Hello World")

# 2. Tạo một đường dẫn (endpoint) dạng GET tại trang chủ "/"
@app.get("/")
async def hello_world():
    # Trả về một Dictionary Python (FastAPI sẽ tự chuyển thành JSON cho trình duyệt hiểu)
    return {
        "message": "Chào mừng bạn đến với bài học FastAPI đầu tiên!",
        "status": "Đang chạy thành công",
        "lesson": 1
    }

# 3. Tạo một đường dẫn khác để test
@app.get("/test")
async def test_route():
    return {"message": "Đường dẫn test này cũng hoạt động rồi!"}
