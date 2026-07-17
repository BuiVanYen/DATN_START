# Hướng Dẫn Kết Nối Samba & Code Chung Trên Raspberry Pi 4

Tài liệu này hướng dẫn thành viên trong nhóm thiết lập kết nối để có thể chỉnh sửa code trực tiếp trên Raspberry Pi 4 thông qua ổ đĩa mạng Windows và làm việc song song hiệu quả.

---

## 📌 BƯỚC 1: Kết nối chung mạng WiFi
Để máy tính của bạn nhìn thấy Raspberry Pi 4, bắt buộc cả **Laptop của bạn** và **Raspberry Pi 4** phải kết nối vào **cùng một mạng WiFi** (hoặc cắm chung dây mạng LAN).

> [!TIP]
> Nếu đi ra ngoài (quán cafe, trường học) mà mạng WiFi chặn kết nối nội bộ, hãy bảo trưởng nhóm bật **Mobile Hotspot** trên Laptop và tất cả kết nối vào đó.

---

## 📌 BƯỚC 2: Map ổ đĩa mạng trên Windows (Map Network Drive)
Thiết lập này giúp đưa thư mục dự án trên Pi thành một ổ đĩa (ổ Z:, Y:...) hiển thị trực tiếp trong máy tính của bạn.

1. Nhấn tổ hợp phím `Windows + E` để mở **File Explorer**.
2. Chọn **This PC** ở cột bên trái.
3. Trên thanh công cụ phía trên đầu, chọn **Map network drive** (Nếu dùng Windows 11, nhấp vào dấu 3 chấm `...` để tìm mục này).
4. Điền cấu hình:
   * **Drive:** Chọn ký tự ổ đĩa tùy ý (Khuyên dùng: **`Z:`**).
   * **Folder:** Nhập địa chỉ mDNS sau:
     ```text
     \\pi.local\DATN_START
     ```
   * **Tích chọn 2 ô:**
     * [x] **`Reconnect at sign-in`** (Tự động kết nối lại khi mở máy).
     * [x] **`Connect using different credentials`** (Yêu cầu nhập tài khoản của Pi).
5. Nhấn **Finish**.

---

## 📌 BƯỚC 3: Nhập thông tin đăng nhập
Một hộp thoại yêu cầu tài khoản sẽ hiện ra, bạn điền như sau:
* **User name:** `pi`
* **Password:** *(Hỏi trưởng nhóm để lấy mật khẩu Samba đã đặt)*
* **Lưu ý:** Tích chọn ô **`Remember my credentials`** (Nhớ thông tin đăng nhập) để lần sau không cần nhập lại.
* Nhấn **OK**. 

*Bây giờ, thư mục dự án trên Pi sẽ xuất hiện như ổ đĩa `Z:` trong mục This PC của bạn.*

---

## 📌 BƯỚC 4: Mở Workspace bằng VS Code
Để vừa xem code trên máy bạn vừa xem/sửa code trên Pi đồng thời:

1. Mở phần mềm **VS Code**.
2. Chọn **File** (Tệp) -> **Open Workspace from File...** (Mở không gian làm việc từ tệp...).
3. Chọn tệp tin **`DATN_START.code-workspace`** trong thư mục dự án cục bộ trên máy bạn.
4. Ở cột bên trái Explorer của VS Code sẽ xuất hiện 2 thư mục:
   * `DATN_START (Laptop)`: Code của bạn dưới local.
   * `DATN_START (Pi4)`: Code thực tế chạy trên Pi 4 (Mọi thay đổi trên đây sẽ lưu trực tiếp vào Pi).

---

## ⚠️ QUY TẮC CODE CHUNG AN TOÀN
Vì đây là thư mục chia sẻ trực tiếp (Real-time Share), hãy tuân thủ các quy tắc sau để không ghi đè làm mất code của nhau:

1. **Phân chia công việc rõ ràng:**
   * Nên làm việc ở các thư mục khác nhau (Ví dụ: Bạn A viết Firmware trong thư mục `firmware/`, Bạn B viết Web API trong thư mục `backend/`).
2. **Không chỉnh sửa cùng một file cùng một lúc:**
   * Nếu cả hai cùng sửa và lưu đè lên một file (ví dụ `main.py`), code của người lưu sau cùng sẽ ghi đè và làm mất code của người lưu trước.
3. **Chạy lệnh trên Pi:**
   * Khi cần chạy lệnh terminal (ví dụ restart Docker: `docker compose restart`), hãy dùng phần mềm SSH (như PowerShell, MobaXterm) để kết nối vào Pi:
     ```bash
     ssh pi@pi.local
     ```

---

## 🛠️ KHẮC PHỤC LỖI TRỄ FILE (LẬP TRÌNH THỜI GIAN THỰC)
Nếu bạn gặp tình trạng **Tạo file trên Pi mà Windows không hiển thị ngay lập tức (bị trễ/cache)**, hãy thực hiện tắt tính năng Cache ổ mạng của Windows theo các bước dưới đây (An toàn 100%):

1. Click chuột phải vào biểu tượng **Start** của Windows -> Chọn **PowerShell (Admin)** hoặc **Terminal (Admin)**.
2. Copy và dán toàn bộ các lệnh dưới đây vào rồi nhấn **Enter**:
   ```powershell
   Set-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Services\LanmanWorkstation\Parameters" -Name "DirectoryCacheLifetime" -Value 0 -Type DWord
   Set-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Services\LanmanWorkstation\Parameters" -Name "FileInfoCacheLifetime" -Value 0 -Type DWord
   Set-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Services\LanmanWorkstation\Parameters" -Name "FileNotFoundCacheLifetime" -Value 0 -Type DWord
   Restart-Service LanmanWorkstation -Force
   ```
3. Sau khoảng 3-5 giây dịch vụ khởi động lại là bạn có thể tạo/sửa file đồng bộ thời gian thực 100% giữa Windows và Pi.
