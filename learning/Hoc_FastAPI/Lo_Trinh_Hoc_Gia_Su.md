# LỘ TRÌNH HỌC TẬP TƯƠNG TÁC CÙNG GIA SƯ AI (ANTIGRAVITY)
> **Đồ án:** Digital Twin Hệ Thống Thủy Canh NFT Xà Lách (XGBoost + FastAPI + OPC UA + NX MCD)

Chào bạn! Tôi sẽ đồng hành cùng bạn với vai trò là **Gia sư riêng** cho đồ án này. Chúng ta sẽ không học lý thuyết suông mà sẽ **học đi đôi với hành**, làm tới đâu hiểu tới đó.

Dưới đây là checklist lộ trình chi tiết từng bước. Mỗi khi hoàn thành một bước, bạn hãy chụp ảnh/báo kết quả cho tôi để chúng ta chuyển sang bài tiếp theo.

---

## 📅 BẢNG LỘ TRÌNH CHI TIẾT

### 🚀 Giai Đoạn 1: Làm Quen Với FastAPI (Thời gian: 2 - 3 ngày)

- [ ] **Bài 1: Cài đặt & Hello World**
  - **Mục tiêu:** Cài đặt thư viện, tạo app FastAPI đầu tiên, khởi chạy server bằng Uvicorn và hiểu cách đọc tài liệu tự động Swagger UI (`/docs`).
  - **Cách thực hành:** Tạo file `test_api.py`, chạy lệnh `uvicorn` và gọi thử API trên trình duyệt.
  - **Câu hỏi cho gia sư:** *"Tôi đã cài xong, làm sao để chạy thử API?"* hoặc *"Giải thích giúp tôi sự khác biệt giữa `@app.get` và `@app.post`?"*

- [ ] **Bài 2: Tạo API Đọc và Gửi dữ liệu (GET & POST)**
  - **Mục tiêu:** Hiểu cách truyền tham số đường dẫn (Path Parameter) và gửi dữ liệu dạng JSON thông qua Pydantic.
  - **Cách thực hành:** Viết API nhận giá trị nhiệt độ, độ ẩm đầu vào và trả về đánh giá "Nóng" hay "Mát".
  - **Câu hỏi cho gia sư:** *"Pydantic là gì và tại sao tôi cần nó để định nghĩa SensorData?"*

---

### 🔗 Giai Đoạn 2: Tích Hợp FastAPI Với OPC UA Server (Thời gian: 3 - 4 ngày)

- [ ] **Bài 3: Đọc Dữ Liệu Cảm Biến Từ Simulator Lên Web**
  - **Mục tiêu:** Dùng `asyncua` Client trong FastAPI để kết nối tới server `opcua_simulator.py` (đang chạy) và lấy dữ liệu cảm biến (nhiệt độ, pH, TDS) trả về qua API GET.
  - **Cách thực hành:** Viết API `/api/sensors/realtime` để khi gọi sẽ trả về các chỉ số từ server ảo.
  - **Câu hỏi cho gia sư:** *"Làm sao để gọi hàm `async` của OPC UA bên trong API FastAPI?"*

- [ ] **Bài 4: API Điều Khiển Thiết Bị Ảo (Quạt/Bơm/Đèn)**
  - **Mục tiêu:** Viết API nhận lệnh từ người dùng (ví dụ: bật quạt 1 tốc độ 200) và ghi giá trị đó trực tiếp vào Node tương ứng trên OPC UA Server.
  - **Cách thực hành:** Tạo API `PUT /api/control/fan` để thay thế cho file `test_fans.py` chạy bằng terminal cũ.
  - **Câu hỏi cho gia sư:** *"Tôi muốn viết API bật/tắt máy bơm chính 220V thì code như thế nào?"*

---

### 🧠 Giai Đoạn 3: Tích Hợp Trí Tuệ Nhân Tạo XGBoost (Thời gian: 3 - 4 ngày)

- [ ] **Bài 5: Huấn Luyện & Lưu Model XGBoost**
  - **Mục tiêu:** Tạo dữ liệu giả lập sinh trưởng xà lách, dùng thư viện `xgboost` để train mô hình hồi quy (Regression) và lưu lại dưới dạng file `.pkl` hoặc `.json`.
  - **Cách thực hành:** Chạy file Python train model trong thư mục `XGBOOST/` để xuất ra file `lettuce_growth_xgboost.pkl`.
  - **Câu hỏi cho gia sư:** *"Làm sao để biết mô hình XGBoost của tôi đã dự đoán tốt hay chưa? R2 score bao nhiêu là đạt?"*

- [ ] **Bài 6: Nhúng Model Vào FastAPI Để Dự Đoán Real-time**
  - **Mục tiêu:** Load file model `.pkl` khi khởi động FastAPI, viết API nhận chỉ số cảm biến hiện tại và trả về % sinh trưởng dự đoán cùng số ngày thu hoạch còn lại.
  - **Cách thực hành:** Viết API `/api/predict-growth` kết hợp với model XGBoost.
  - **Câu hỏi cho gia sư:** *"Làm thế nào để truyền dữ liệu từ OPC UA trực tiếp vào model XGBoost để dự đoán tự động mỗi 5 giây?"*

---

### 🌐 Giai Đoạn 4: Hoàn Thiện Digital Twin & Real-time (Thời gian: 2 - 3 ngày)

- [ ] **Bài 7: Truyền Dữ Liệu Real-time Bằng WebSocket**
  - **Mục tiêu:** Thay vì Web App phải gọi API liên tục để cập nhật chỉ số cảm biến, ta dùng WebSocket để FastAPI tự động đẩy dữ liệu cảm biến mới nhất về giao diện mỗi giây.
  - **Cách thực hành:** Tạo kết nối WebSocket `/ws/live-data` kết nối tới OPC UA Server.
  - **Câu hỏi cho gia sư:** *"WebSocket khác gì so với HTTP GET thông thường và làm sao để test thử nó?"*

- [ ] **Bài 8: Kiểm Thử Toàn Diện Hệ Thống (Digital Twin Integration)**
  - **Mục tiêu:** Chạy đồng thời: Simulator OPC UA + FastAPI (XGBoost) + Siemens NX MCD. Kiểm tra xem khi thay đổi thông số trên API/Web thì mô hình 3D trong NX MCD có chuyển động/thay đổi kích thước tương ứng không.
  - **Câu hỏi cho gia sư:** *"Tôi chạy tất cả nhưng cây trong NX MCD không lớn lên, làm sao để debug lỗi kết nối?"*

---

## 👨‍🏫 NGUYÊN TẮC HỌC CÙNG GIA SƯ AI

1. **Học từng bài một:** Đừng vội vàng nhảy cóc. Hãy hoàn thành tốt Bài 1 trước khi sang Bài 2.
2. **Hỏi bất cứ khi nào gặp lỗi:** Khi code bị báo lỗi (màu đỏ ở terminal), hãy copy toàn bộ đoạn lỗi đó và gửi cho tôi. Tôi sẽ giải thích tại sao lỗi và hướng dẫn bạn sửa.
3. **Chủ động yêu cầu bài tập:** Sau mỗi bài, nếu bạn muốn luyện tập thêm, hãy nói: *"Hãy cho tôi một bài tập nhỏ về phần này"*.

---

### 🏁 BẮT ĐẦU NGAY BÂY GIỜ!
Bạn đã sẵn sàng chưa? Nếu đã sẵn sàng, hãy gõ:
👉 **"Bắt đầu Bài 1: Cài đặt và Hello World"** để tôi hướng dẫn bạn viết dòng code FastAPI đầu tiên nhé!
