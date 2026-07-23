# Hướng Dẫn Thiết Lập & Xem Webcam Từ Xa Cho Đồ Án

Tài liệu này hướng dẫn cách cấu hình, chạy hệ thống truyền phát (stream) Webcam thời gian thực từ Raspberry Pi 4 để các thành viên có thể lập trình và quan sát mô hình từ xa từ nhà riêng.

---

## 📌 PHẦN 1: Chạy Thử Nghiệm Trong Mạng Nội Bộ (Chung WiFi ở nhà)

Trước khi cấu hình đi xa, bạn nên kiểm tra xem Camera hoạt động tốt với Pi và code Python chưa.

### 1. Chuẩn bị kết nối cứng trên Pi 4
* Cắm **ổ cứng SSD** (hệ điều hành) vào **cổng USB 3.0 (màu xanh dương)**.
* Cắm **Webcam USB (Xiaomi Xiaovv)** vào **cổng USB 2.0 (màu đen)**.
  > [!IMPORTANT]  
  > **Lưu ý về nguồn điện:** Tránh cắm chung webcam với quá nhiều thiết bị USB công suất lớn khác trực tiếp trên Pi để tránh hiện tượng sụt nguồn làm ngắt SSD (gây lỗi `Input/output error` treo hệ điều hành).

### 2. Chạy Server Stream Camera
Mở Terminal của Pi (qua SSH hoặc VNC) và chạy các lệnh sau:
```bash
cd ~/DATN_START/webcam
python3 webcam_stream.py
```
Nếu màn hình Terminal hiển thị:
`Server đang chạy tại http://localhost:8000/video_feed (dùng Pure Python)` nghĩa là server đã khởi động thành công.

### 3. Xem hình ảnh trên Laptop
Mở trình duyệt Web (Chrome/Edge/Firefox) trên Laptop đang kết nối chung WiFi với Pi và truy cập địa chỉ:
* **Địa chỉ 1 (Dùng mDNS):** `http://pi.local:8000/video_feed`
* **Địa chỉ 2 (Dùng IP trực tiếp):** `http://<IP_CỦA_PI>:8000/video_feed` (Ví dụ: `http://192.168.1.15:8000/video_feed`)
  *(Xem IP của Pi bằng cách chạy lệnh `hostname -I` trên terminal của Pi).*

---

## 📌 PHẦN 2: Thiết Lập Kết Nối & Lập Trình Từ Xa (Khi về nhà riêng)

Khi bạn về nhà riêng, bạn không còn chung mạng WiFi với mô hình nữa. Hãy làm theo 3 bước sau:

### Bước 1: Tạo cầu nối mạng bằng Tailscale VPN
Dịch vụ này giúp đưa Laptop của bạn và Pi 4 ở nhà bạn vào cùng một mạng ảo bảo mật mà không cần mở cổng modem.

1. Đăng ký tài khoản miễn phí tại: [https://tailscale.com/](https://tailscale.com/)
2. Cài đặt trên **Raspberry Pi 4**:
   ```bash
   curl -fsSL https://tailscale.com/install.sh | sh
   sudo tailscale up
   ```
   *Nhấp vào đường link hiển thị trên màn hình để đăng nhập và liên kết Pi vào tài khoản.*
3. Cài đặt phần mềm **Tailscale** trên Laptop của bạn (chọn bản cho Windows/macOS) và đăng nhập chung tài khoản.
4. Truy cập bảng điều khiển (Dashboard) của Tailscale để lấy địa chỉ **IP Tailscale của Pi** (Dạng: `100.x.y.z`, ví dụ: `100.115.80.20`). IP này là cố định cho Pi.

### Bước 2: Lập trình từ xa bằng VS Code (Remote - SSH)
Vì Samba qua internet rất lag, ta dùng extension Remote-SSH để code trực tiếp trên SSD của Pi với tốc độ siêu nhanh.

1. Mở **VS Code** trên Laptop của bạn.
2. Tìm và cài đặt extension: **`Remote - SSH`** (do Microsoft phát triển).
3. Nhấn nút màu xanh lá nhỏ ở góc dưới cùng bên trái VS Code (hoặc nhấn `F1` -> gõ `Remote-SSH: Connect to Host...`).
4. Chọn **Add New SSH Host** và nhập:
   ```text
   ssh pi@<Địa_chỉ_IP_Tailscale_của_Pi>
   ```
5. Chọn file cấu hình mặc định để lưu lại, nhấn **Connect** và nhập mật khẩu của user `pi`.
6. Chọn **Open Folder** và trỏ tới thư mục `/home/pi/DATN_START`. Giờ bạn có thể viết code trực tiếp trên Pi như đang ngồi cạnh nó!

### Bước 3: Xem Webcam từ xa qua mạng Tailscale
1. Mở terminal SSH trên VS Code, khởi chạy camera:
   ```bash
   python3 ~/DATN_START/webcam/webcam_stream.py
   ```
2. Trên trình duyệt Laptop của bạn ở nhà, truy cập đường dẫn sau để xem trực tiếp:
   ```text
   http://<Địa_chỉ_IP_Tailscale_của_Pi>:8000/video_feed
   ```

---

## 📌 PHẦN 3: Chạy Ngầm Server Camera (Background Service)

Khi bạn muốn tắt máy tính đi ngủ hoặc đóng Terminal VS Code mà Camera **vẫn tiếp tục chạy ngầm** trên Pi để người khác xem:

### 1. Kích hoạt chạy ngầm:
```bash
nohup python3 ~/DATN_START/webcam/webcam_stream.py > ~/DATN_START/webcam/webcam.log 2>&1 &
```
*Lệnh này sẽ chuyển hướng log hoạt động vào file `webcam.log` và chạy ẩn dưới nền.*

### 2. Kiểm tra xem Server ngầm đang chạy không:
```bash
ps aux | grep webcam_stream
```

### 3. Tắt tiến trình chạy ngầm:
```bash
pkill -f webcam_stream.py
```

---

## 🛠️ Xử Lý Lỗi Thường Gặp (Troubleshooting)

### 1. Báo lỗi `Input/output error` hoặc `sudo: command not found`
* **Triệu chứng:** Gõ bất kỳ lệnh nào cũng bị báo lỗi, đĩa cứng không cho ghi file.
* **Nguyên nhân:** Ổ SSD bị sụt áp nguồn cấp do camera ngốn dòng quá lớn đột ngột, hệ điều hành tự động khóa đĩa lại ở chế độ `Read-Only` để tránh hỏng dữ liệu.
* **Cách khắc phục:** 
  1. Rút phích cắm nguồn Type-C của Pi ra.
  2. Cắm lại camera sang cổng màu đen (USB 2.0), ổ SSD cắm sang cổng màu xanh (USB 3.0).
  3. Cắm lại nguồn Pi. Nếu vẫn bị thường xuyên, bạn bắt buộc phải mua một **Hub USB có nguồn ngoài** để cắm SSD hoặc dùng cục nguồn nguồn Pi 4 chất lượng cao hơn (đúng 5V - 3A).

### 2. Không truy cập được trang livestream
* Đảm bảo app Tailscale trên cả Laptop và Pi đang ở trạng thái **Connected** (Connected màu xanh lá).
* Đảm bảo bạn nhập đúng cổng `:8000` và đúng endpoint `/video_feed` ở cuối link.
